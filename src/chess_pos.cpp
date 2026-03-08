#include <cstring>
#include <algorithm>
#include "chess_pos.h"
#include "bitops.h"
#include "chess_funcs.h"

#define U64 uint64_t

using namespace std;

chess_mask_LUT chess_pos::MLUT;

chess_pos::chess_pos(){
	next = nullptr;
	prev = nullptr;
}

uint16_t chess_pos::operator - (chess_pos const &c1){
	uint16_t move; 
	chess_pos p1, p2;
	p1 = c1;
	p1.next = &p2;
	p1.generate_moves();
	while((move = p1.pop_and_add())){
		if (p2 == *this) {
			break;
		}
	}
	return move;
}

bool chess_pos::operator == (chess_pos const &c1){
	int i,j;

	for(i=0;i<2;i++){
		for(j=0;j<6;j++){
			if(c1.pieces[i][j] != this->pieces[i][j]) return false;
		}
	}
	if(c1.ep_target_square != this->ep_target_square) return false;
	if(c1.castlable_rooks != this->castlable_rooks) return false;
	if(c1.to_move != this->to_move) return false;
	return true;
}

void chess_pos::copy_pos(const chess_pos& source_pos){
	memcpy(pl, source_pos.pl, sizeof(pl));
	memcpy(pin_rays, source_pos.pin_rays, sizeof(pin_rays));
	memcpy(pieces, source_pos.pieces, sizeof(pieces));
	memcpy(occ, source_pos.occ, sizeof(occ));
	memcpy(piece_at, source_pos.piece_at, sizeof(piece_at));

	zobrist_key = source_pos.zobrist_key;
	ep_target_square = source_pos.ep_target_square;
	castlable_rooks = source_pos.castlable_rooks; 
	to_move = source_pos.to_move;
	last_move_null = source_pos.last_move_null;
	changed_squares = source_pos.changed_squares;
	last_move_to_and_from = source_pos.last_move_to_and_from;
}

void chess_pos::sort_piece_list()
{
	int i,j,side,min,min_idx,p_type;
	U64 min_loc;
	piece_list_struct pl_temp;
	for(side=WHITE;side<=BLACK;side++){
		for(j=bitops::popcount64(occ[side])-1;j>0;j--){
			min = KING;
			for(i=j;i>0;i--){
				p_type = pl[side][i].piece_type;
				if(p_type < min){
					min = p_type;
					min_loc = pl[side][i].loc;
					min_idx = i;
				} else if(p_type == min){
					if((pl[side][i].loc < min_loc) && side){
						min_loc = pl[side][i].loc;
						min_idx = i;
					} else if((pl[side][i].loc > min_loc) && !side){
						min_loc = pl[side][i].loc;
						min_idx = i;
					}
				}
			}
			pl_temp = pl[side][min_idx];
			pl[side][min_idx] = pl[side][j];
			pl[side][j] = pl_temp;
		}
	}
}


void chess_pos::load_new_fen(string FEN)
{
	int i,j,k,n;
	char piece_chars[2][6] = {{'P','N','B','R','Q','K'},{'p','n','b','r','q','k'}};
	unsigned long idx;
	U64 u;

	string position = FEN;

	for(i=0;i<12;i++){
		pieces[i/6][i%6] = 0;
		j = 0;
		while(!isspace(position[j])){
			//cout << position[j];
			pieces[i/6][i%6] <<= 1;
			if(position[j] == '/'){
				pieces[i/6][i%6] >>= 1;
			} else if(isdigit(position[j])){
				n = atoi(&position[j]);
				pieces[i/6][i%6] <<= n - 1;
			} else if(position[j] == piece_chars[i/6][i%6]){
				pieces[i/6][i%6] += 1;
			}

			j++;
		}
	}
	j++;

	for(i=0;i<64;i++) piece_at[i] = 0;

	for(i=0;i<6;i++){
		for(k=0;k<2;k++){
			u = pieces[k][i];
			while(bitops::bscanf64( &idx, u)){
				u ^= (U64)1 << idx; 
				piece_at[idx] = i;
			}	
		}
	}


	to_move = 0;
	if(position[j++] == 'b') to_move = 1;
	j++;

	castlable_rooks = 0;
	if(position[j] != '-'){
		while(position[j] != ' '){
			if(position[j] == 'K'){
				castlable_rooks += pieces[WHITE][ROOK] & MLUT.get_straight_ray(0,bit_to_idx(pieces[WHITE][KING]));
			} else if(position[j] == 'Q'){
				castlable_rooks += pieces[WHITE][ROOK] & MLUT.get_straight_ray(2,bit_to_idx(pieces[WHITE][KING]));
			} else if(position[j] == 'k'){
				castlable_rooks += pieces[BLACK][ROOK] & MLUT.get_straight_ray(0,bit_to_idx(pieces[BLACK][KING]));
			} else if(position[j] == 'q'){
				castlable_rooks += pieces[BLACK][ROOK] & MLUT.get_straight_ray(2,bit_to_idx(pieces[BLACK][KING]));
			}
			j++;
		}
	} else {
		j++;
	}
	j++;

	ep_target_square = 0;
	if(position[j] != '-'){
		ep_target_square = (U64)1;
		ep_target_square <<= 7-(int(position[j])-97);
		j++;
		ep_target_square <<= 8*(atoi(&position[j])-1);
	}

	occ[0] = occ[1] = 0;
	for(i=0;i<6;i++){
		occ[0] |= pieces[0][i];
		occ[1] |= pieces[1][i];
	}
	for(i=0;i<8;i++){
		pin_rays[0][i] = 0;
		pin_rays[1][i] = 0;
	}
	last_move_check_evasion = 0;
	last_move_capture = 0;
	last_move_to_and_from = 0;
	changed_squares = occ[!to_move];

	for(i=0;i<MAX_PIECES_PER_SIDE;i++){
		for(j=0;j<2;j++){
			pl[j][i].piece_type = 0;
			pl[j][i].pinned = 0;
			pl[j][i].loc = 0;
			pl[j][i].ctrl_sq = 0;
			pl[j][i].targets = 0;
		}
	}

	init_piece_list();
	sort_piece_list();
	init_zobrist();
}

chess_pos::chess_pos(string FEN)
{
	next = nullptr;
	prev = nullptr;
	load_new_fen(FEN);
}

void chess_pos::add_move_to_next_node(uint16_t move)
{
	next->copy_pos(*this);
	next->add_move(move);
}

int chess_pos::piece_at_idx(int idx, int side)
{
	int i;

	for(i=0;i<6;i++){
		if( ((pieces[side][i] >> idx) & (U64)1) == 1 ) return i;
	}

	return -1;
}


U64 chess_pos::prune_blocked_moves(int piece_type, int move_mask_center_idx, U64* move_mask, U64 blockers)
{

	U64 u,actual_blockers = 0;
	unsigned long idx;

	u = (*move_mask & blockers);

	//leaving these loops unrolled
	if(u){
		if(piece_type == ROOK){
			//ROOK
			if (bitops::bscanr64( &idx, u & MLUT.get_straight_ray(0,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(0,idx);
			}
			if (bitops::bscanr64( &idx, u & MLUT.get_straight_ray(1,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(1,idx);
			} 
			if (bitops::bscanf64( &idx, u & MLUT.get_straight_ray(2,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(2,idx);
			} 
			if (bitops::bscanf64( &idx, u & MLUT.get_straight_ray(3,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(3,idx);
			} 
		} else if(piece_type == BISHOP){
			//BISHOP
			if (bitops::bscanr64( &idx, u & MLUT.get_diag_ray(0,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(0,idx);
			}
			if (bitops::bscanr64( &idx, u & MLUT.get_diag_ray(1,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(1,idx);
			} 
			if (bitops::bscanf64( &idx, u & MLUT.get_diag_ray(2,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(2,idx);
			} 
			if (bitops::bscanf64( &idx, u & MLUT.get_diag_ray(3,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(3,idx);
			} 
		} else if(piece_type == QUEEN){
			// Queen: combines straight (rook) and diagonal (bishop) rays
			// Straight rays (E, S, W, N)
			if (bitops::bscanr64( &idx, u & MLUT.get_straight_ray(0,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(0,idx);
			}
			if (bitops::bscanr64( &idx, u & MLUT.get_straight_ray(1,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(1,idx);
			}
			if (bitops::bscanf64( &idx, u & MLUT.get_straight_ray(2,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(2,idx);
			}
			if (bitops::bscanf64( &idx, u & MLUT.get_straight_ray(3,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_straight_ray(3,idx);
			}
			// Diagonal rays (SE, SW, NW, NE)
			if (bitops::bscanr64( &idx, u & MLUT.get_diag_ray(0,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(0,idx);
			}
			if (bitops::bscanr64( &idx, u & MLUT.get_diag_ray(1,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(1,idx);
			}
			if (bitops::bscanf64( &idx, u & MLUT.get_diag_ray(2,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(2,idx);
			}
			if (bitops::bscanf64( &idx, u & MLUT.get_diag_ray(3,move_mask_center_idx))){
				actual_blockers |= (U64)1 << idx;
				*move_mask &= ~MLUT.get_diag_ray(3,idx);
			}
		} else {
			actual_blockers = *move_mask & blockers;
		}
	}

	*move_mask &= ~blockers;

	return actual_blockers;

}

void chess_pos::init_zobrist()
{
	size_t i, side;
	int p_type, idx;

	zobrist_key = 0;

	for(side=WHITE;side<=BLACK;side++){
		for(i=0;i<bitops::popcount64(occ[side]);i++){
			p_type = pl[side][i].piece_type;
			idx = bit_to_idx(pl[side][i].loc);
			zobrist_key ^= MLUT.get_zobrist_piece(side, p_type, idx);
		}
	}
	zobrist_key ^= z_key(ep_target_square);
	zobrist_key ^= z_key(castlable_rooks);
	if(to_move) zobrist_key ^= MLUT.get_zobrist_btm();
}

void chess_pos::init_piece_list()
{
	U64 attackers;
	U64 attacker_loc,king_loc;
	int side, curr_piece, attacking_piece;
	U64 enemy_king_loc;
	unsigned long idx;
	U64 u,m,u_temp, straight_blockers=0;
	U64 diag_blockers=0;
	int king_idx;

	for(side = WHITE;side<=BLACK;side++){
		curr_piece = 0;
		attackers = occ[side];
		king_loc = pieces[side][KING];
		enemy_king_loc = pieces[!side][KING];
		king_idx = bit_to_idx(king_loc);
		straight_blockers = pieces[!side][QUEEN] | pieces[!side][ROOK];
		diag_blockers = pieces[!side][QUEEN] | pieces[!side][BISHOP];

		pl[side][0].piece_type = KING;
		pl[side][0].loc = king_loc;

		u = MLUT.get_move_mask(KING,king_idx);
		u_temp = prune_blocked_moves(KING, king_idx, &u, occ[0] | occ[1]);
		u |= u_temp;

		pl[side][0].ctrl_sq = u;

		while(bitops::bscanf64( &idx, attackers ^ king_loc)){
			curr_piece++;
			attacker_loc = (U64)1<<idx;
			attackers ^= attacker_loc;
			attacking_piece = piece_at[idx];

			pl[side][curr_piece].piece_type = attacking_piece;
			pl[side][curr_piece].loc = attacker_loc;

			if(attacking_piece == PAWN){
				u = (MLUT.get_pawn_attack_mask(side,idx));
			} else {
				u = MLUT.get_move_mask(attacking_piece,idx);
				u_temp = prune_blocked_moves(attacking_piece, idx, &u, (occ[0] | occ[1]) & ~enemy_king_loc);
				u |= u_temp;
			}
			//u &= ~attacker_loc;

			pl[side][curr_piece].ctrl_sq = u;

		}
		init_targets(side);

		const int straight_directions[4] = {E_DIR,S_DIR,W_DIR,N_DIR};
		const int diag_directions[4] = {SE_DIR,SW_DIR,NW_DIR,NE_DIR};
		for(int straight = 1; straight >= 0; straight--){
			u = straight ? straight_blockers : diag_blockers;	
			for(int it=0;it<4;it++){
				m = straight ? MLUT.get_straight_ray(it,king_idx) : MLUT.get_diag_ray(it,king_idx);
				bool blocked = it < 2 ? bitops::bscanr64( &idx, u & m) : bitops::bscanf64( &idx, u & m);
				if(blocked){
					m &= straight ? ~MLUT.get_straight_ray(it,idx) : ~MLUT.get_diag_ray(it,idx);
					m |= ((U64)1<<idx);
				}
				pin_rays[side][straight ? straight_directions[it] : diag_directions[it]] = m;
			}
		}
	}
}

void chess_pos::store_init_targets(U64 piece_loc, U64 targets, int pinned)
{
	//does not need to be fast
	int i,side, num_pieces;
	targets &= ~piece_loc;
	for(side = WHITE;side<=BLACK;side++){
		num_pieces = bitops::popcount64(occ[side]);
		for(i=0;i<num_pieces;i++){
			if(pl[side][i].loc == piece_loc){
				pl[side][i].targets = targets;
				pl[side][i].pinned = 0;
				if(pinned) pl[side][i].pinned = 1;
				return;
			}
		}
	}
}

void chess_pos::init_targets(int side)
{
	//this is essentially the old move generation code modified to store moves in the piece list struct
	//speed is not of concern here
	U64 enemies = occ[!side]; //enemies gets modified
	U64 allies = occ[side];
	U64 allied_pawns = pieces[side][PAWN];
	unsigned long idx, pinned_idx;
	int attacking_piece, pinned_piece;
	U64 u, m, u_temp, v, ab_ray, attacker_loc, king_loc, possible_pins, e_controlled_sq;
	U64 ep_target_square_copy = ep_target_square;
	U64 pinned_slider_moves[8][2];
	int pinned_sliders = 0;
	U64 pinned_pawn_moves[8][2];
	int pinned_pawns = 0;


	in_check = 0;
	king_loc = pieces[side][KING];

	e_controlled_sq = 0;

	mList.reset_num_moves();	

	allies &= ~allied_pawns;

	//check for special case of en passant pin
	if(ep_target_square > 0){
		if(king_loc & EN_PASSANT_ATTACKER_RANKS){
			u = MLUT.get_en_passant_attackers(bit_to_idx(ep_target_square)) & allied_pawns;
			if(bit_to_idx(king_loc)/8 == bit_to_idx(u)/8){
				m = (pieces[!side][ROOK]+pieces[!side][QUEEN]);
				if(king_loc > u){
					//king left of en passant attackers
					u_temp = MLUT.get_straight_ray(0,bit_to_idx(king_loc));
					if(bitops::bscanr64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(0,idx);
						if(bitops::popcount64(u_temp & (occ[0] | occ[1])) == 3){
							ep_target_square = 0;
						}
					}
				} else {
					//king right of en passant attackers
					u_temp = MLUT.get_straight_ray(2,bit_to_idx(king_loc));
					if(bitops::bscanf64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(2,idx);
						if(bitops::popcount64(u_temp & (occ[0] | occ[1])) == 3){
							ep_target_square = 0;
						}
					}
				}
			}
		}
	}

	//find squares attacked by enemy and allied pieces that are pinned
	while(bitops::bscanf64( &idx, enemies)){
		attacker_loc = (U64)1<<idx;
		enemies ^= attacker_loc;
		attacking_piece = piece_at[idx];

		if (attacking_piece==PAWN){
			u = MLUT.get_pawn_attack_mask(!side,idx);
		} else {
			u = MLUT.get_move_mask(attacking_piece,idx);
		}

		//prune enemy's moves blocked by same color pieces
		u_temp = prune_blocked_moves(attacking_piece, idx, &u, occ[!side]);

		//if xraying check for pinned pieces
		if(is_sliding_piece(attacking_piece)){
			if((king_loc & u) > 0 ) {
				ab_ray = MLUT.get_sliding_ray( king_loc | attacker_loc ); 
				possible_pins = (ab_ray & occ[side]) ^ king_loc;
				if(possible_pins == 0){
					//if no possible pinned pieces the king is attacked
					in_check++;
				} else if( (bitops::popcount64(possible_pins) == 1) ){
					//check for obstructions, otherwise a piece is pinned
					pinned_idx = bit_to_idx(possible_pins);
					pinned_piece = piece_at[pinned_idx];
					//GENERATE PINNED PIECE MOVES
					if (pinned_piece==PAWN){
						allied_pawns &= ~possible_pins;
						v = (occ[0] | occ[1]) ^ possible_pins;
						if(side){
							v |= v >> 8;
						} else {
							v |= v << 8;
						}
						m = MLUT.get_pawn_attack_mask(side,pinned_idx) & (occ[!side] | ep_target_square);
						m |= (MLUT.get_move_mask(PAWN+B_PAWN*side,pinned_idx) & ~v);
						m &= ab_ray ^ king_loc;
						pinned_pawn_moves[pinned_pawns][0] = possible_pins;
						pinned_pawn_moves[pinned_pawns++][1] = m;
					} else if(true/*pinned_piece != KNIGHT*/){
						//sliding piece. store moves later in case another check is found
						allies ^= possible_pins;
						m = MLUT.get_move_mask(pinned_piece,pinned_idx);
						m &= ab_ray ^ king_loc;
						pinned_slider_moves[pinned_sliders][0] = possible_pins;
						pinned_slider_moves[pinned_sliders++][1] = m;
					} else {
						allies &= ~possible_pins;
					}
				}
				

			}
		} else {
			if((king_loc & u) > 0 ) {
				in_check++;
			}
		}

		//prune enemy's moves blocked by different color pieces
		u |= u_temp;
		prune_blocked_moves(attacking_piece, idx, &u, occ[side] ^ king_loc);

		e_controlled_sq |=  u;
		u &= ~u_temp;
	}

	//generate moves for side to move
	allies ^= king_loc;
	//PAWNS first
	while(bitops::bscanf64( &idx, allied_pawns)){
		attacker_loc = (U64)1<<idx;
		allied_pawns ^= attacker_loc;

		u = create_pawn_pushes(attacker_loc,side);
		u |= MLUT.get_pawn_attack_mask(side,idx);

		store_init_targets(attacker_loc,u,false);
	}
	//pinned pawns
	while(pinned_pawns-- > 0){
		attacker_loc = pinned_pawn_moves[pinned_pawns][0];
		u = pinned_pawn_moves[pinned_pawns][1];
		store_init_targets(attacker_loc,u,true);
	}

	//other pieces next except for the king
	while(bitops::bscanf64( &idx, allies)){
		attacker_loc = (U64)1<<idx;
		allies ^= attacker_loc;
		attacking_piece = piece_at[idx];

		u = MLUT.get_move_mask(attacking_piece,idx);		
		u_temp = prune_blocked_moves(attacking_piece, idx, &u, (occ[0] | occ[1]));
		u |= u_temp;

		store_init_targets(attacker_loc,u,false);

	}
	//pinned sliders
	while(pinned_sliders-- > 0){
		attacker_loc = pinned_slider_moves[pinned_sliders][0];
		u = pinned_slider_moves[pinned_sliders][1];
		store_init_targets(attacker_loc,u,true);
	}

	//lastly the KING moves
	bitops::bscanf64( &idx, king_loc);

	u = MLUT.get_move_mask(KING,idx);		
	u_temp = prune_blocked_moves(KING, idx, &u, (occ[0] | occ[1]));
	u |= u_temp;

	store_init_targets(king_loc,u,false);

	ep_target_square = ep_target_square_copy;
}

void chess_pos::generate_moves()
{
	const int straight_directions[4] = {E_DIR,S_DIR,W_DIR,N_DIR};
	const int diag_directions[4] = {SE_DIR,SW_DIR,NW_DIR,NE_DIR};
	U64 king_loc, e_king_loc, straight_blockers, diag_blockers;
	U64 u,m,v,u_temp,possible_pins, unpins=0;
	U64 actual_pins[2] = {0};
	U64 partial_pins[2] = {0}; //simply the cumulative of the pin rays that were added/changed from last move
	uint32_t num_pieces[2];
	U64 ally_ctrl = 0;
	U64 enem_ctrl = 0;
	U64 ally_targ = 0;
	U64 enem_targ = 0;
	U64 king_attacker_ray = 0xFFFFFFFFFFFFFFFF;
	U64 can_castle;
	U64 ep_sq = ep_target_square;
	uint32_t i,j, king_idx, e_king_idx,piece_type,was_pinned;
	unsigned long idx;
	uint16_t move, src_square;
	bool ep_check = false;
	mList.reset_num_moves();

	in_check = 0;
	if(changed_squares == 0)changed_squares = occ[!to_move];

	num_pieces[to_move] = bitops::popcount64(occ[to_move]);
	num_pieces[!to_move] = bitops::popcount64(occ[!to_move]);

	// load white king position. 
	// if squares changed in a pin ray, find the new pin ray for that direction
	// see which pieces are actually pinned on the rays and store loc in actual pins

	king_loc = pieces[to_move][KING];
	king_idx = bit_to_idx(king_loc);
	e_king_loc = pieces[!to_move][KING];
	e_king_idx = bit_to_idx(e_king_loc);

	// Phase 1: Update pin rays for the side to move's king (incremental - only rays touched by changed_squares).
	//check for special case of en passant pin
	//only checks pins from straight rays
	//pin from diag rays (i.e. bishop) is NOT checked as this sort of pin cannot occur naturally
	//this will lead to crashes in certain impossible positions e.g. "8/8/2k5/8/4Pp2/8/6B1/4K3 b - e3 0 1"
	if(ep_sq > 0){
		if(king_loc & EN_PASSANT_ATTACKER_RANKS){
			u = MLUT.get_en_passant_attackers(bit_to_idx(ep_sq)) & pieces[to_move][PAWN];
			if(king_idx/8 == bit_to_idx(u)/8){
				m = (pieces[!to_move][ROOK]+pieces[!to_move][QUEEN]);
				if(king_loc > u){
					//king left of en passant attackers
					u_temp = MLUT.get_straight_ray(0,king_idx);
					if(bitops::bscanr64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(0,idx);
						if(bitops::popcount64(u_temp & (occ[0] | occ[1])) == 3){
							ep_sq = 0;
						}
					}
				} else {
					//king right of en passant attackers
					u_temp = MLUT.get_straight_ray(2,king_idx);
					if(bitops::bscanf64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(2,idx);
						if(bitops::popcount64(u_temp & (occ[0] | occ[1])) == 3){
							ep_sq = 0;
						}
					}
				}
			}
		}
	}

	straight_blockers = pieces[!to_move][QUEEN] | pieces[!to_move][ROOK];
	diag_blockers = pieces[!to_move][QUEEN] | pieces[!to_move][BISHOP];

	u = straight_blockers;

	v = occ[to_move] & ~king_loc;
	u_temp = occ[!to_move];

	for(int straight = 1; straight >= 0; straight--){
		u = straight ? straight_blockers : diag_blockers;
		for(int it=0;it<4;it++){
			int direction = straight ? straight_directions[it] : diag_directions[it];
			if(changed_squares & pin_rays[to_move][direction]){
				m = straight ? MLUT.get_straight_ray(it,king_idx) : MLUT.get_diag_ray(it,king_idx);
				bool blocked = (it < 2) ? bitops::bscanr64( &idx, u & m) : bitops::bscanf64( &idx, u & m);
				if (blocked){
					m &= straight ? ~MLUT.get_straight_ray(it,idx) : ~MLUT.get_diag_ray(it,idx);
					if((m & u_temp)==0){
						possible_pins = m & v;
						m |= ((U64)1<<idx);
						j = bitops::popcount64(possible_pins);
						if(j == 1){
							actual_pins[to_move] |= possible_pins;
						}else if( j == 0){
							in_check++;
							king_attacker_ray = m;
						} 
					} else {
						m |= ((U64)1<<idx);
					}
				}
				unpins |= (pin_rays[to_move][direction] & ~m);
				partial_pins[to_move] |= m;
				pin_rays[to_move][direction] = m;
			}
		}
	}

	// Phase 2: Update pin rays for the enemy king.
	// load black king position.
	// save pin rays if last move moved to or from a pin ray.
	// see which pieces are actually pinned on the rays.

	straight_blockers = pieces[to_move][QUEEN] | pieces[to_move][ROOK];
	diag_blockers = pieces[to_move][QUEEN] | pieces[to_move][BISHOP];

	v = occ[!to_move] & ~e_king_loc;

	u_temp = occ[to_move];


	// if the last move was a king move, pin rays are regenerated in all directions.
	if(e_king_loc & changed_squares){

		for(int straight = 1; straight >= 0; straight--){
			u = straight ? straight_blockers : diag_blockers;	
			for(int it=0;it<4;it++){
				m = straight ? MLUT.get_straight_ray(it,e_king_idx) : MLUT.get_diag_ray(it,e_king_idx);
				bool blocked = (it < 2) ? bitops::bscanr64( &idx, u & m) : bitops::bscanf64( &idx, u & m);
				if (blocked){
					m &= straight ? ~MLUT.get_straight_ray(it,idx) : ~MLUT.get_diag_ray(it,idx);
					possible_pins = m & v;
					if(((m&u_temp)==0) && (bitops::popcount64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
					m |= ((U64)1<<idx);
					partial_pins[!to_move] |= m;
				}
				pin_rays[!to_move][straight ? straight_directions[it] : diag_directions[it]] = m;
			}
		}

	} else {

		for(int straight = 1; straight >= 0; straight--){
			u = straight ? straight_blockers : diag_blockers;	
			for(int it=0;it<4;it++){
				int direction = straight ? straight_directions[it] : diag_directions[it];
				if(changed_squares & pin_rays[!to_move][direction]){
					m = straight ? MLUT.get_straight_ray(it,e_king_idx) : MLUT.get_diag_ray(it,e_king_idx);
					bool blocked = (it < 2) ? bitops::bscanr64( &idx, u & m) : bitops::bscanf64( &idx, u & m);
					if (blocked){
						m &= straight ? ~MLUT.get_straight_ray(it,idx) : ~MLUT.get_diag_ray(it,idx);
						possible_pins = m & v;
						if(((m&u_temp)==0) && (bitops::popcount64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
						m |= ((U64)1<<idx);
					}
					pin_rays[!to_move][direction] = m;
					partial_pins[!to_move] |= m;
				}
			}
		}
	}


	// Phase 3: Update move targets for side-to-move pieces affected by the last move or pin changes.
	// iterate through white non-king pieces,
	// generate moves if last move changed them or the piece is in a new white king pin ray
	// if theres an enpassant target, generate moves of en passant attackers.
	// if the piece is not there its been captured. remove it from the list.

	m = (occ[0] | occ[1]) & ~e_king_loc;
	for(i=1;i<num_pieces[to_move];i++){
		U64& piece_loc = pl[to_move][i].loc;
		if(piece_loc & changed_squares){ //piece was captured
			pl[to_move][i] = pl[to_move][num_pieces[to_move]];
		}
		piece_type = pl[to_move][i].piece_type;
		U64& piece_targets = pl[to_move][i].targets;
		U64& piece_control = pl[to_move][i].ctrl_sq;
		was_pinned = pl[to_move][i].pinned;
		if(piece_loc & partial_pins[to_move]){ //piece is on a partial pin ray that changed, generate moves for this piece
			idx = bit_to_idx(piece_loc);
			if(piece_type != PAWN){
				if(piece_type == ROOK)
					piece_control = MLUT.get_rook_attacks(idx, m);
				else if(piece_type == BISHOP)
					piece_control = MLUT.get_bishop_attacks(idx, m);
				else if(piece_type == QUEEN)
					piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
				else
					piece_control = MLUT.get_move_mask(piece_type, idx);
				if(piece_loc & actual_pins[to_move]){
					pl[to_move][i].pinned = 1;
					piece_targets = piece_control & pin_rays[to_move][get_ray_dir(king_idx,idx)];
				} else {
					pl[to_move][i].pinned = 0;
					piece_targets = piece_control;
				}
			} else {
				u = MLUT.get_pawn_attack_mask(to_move,idx);
				piece_control = u;
				u |= create_pawn_pushes(piece_loc, to_move);
				piece_targets = u;
				if(piece_loc & actual_pins[to_move]){
					pl[to_move][i].pinned = 1;
					piece_targets = piece_targets & pin_rays[to_move][get_ray_dir(king_idx,idx)];
				} else {
					pl[to_move][i].pinned = 0;
				}
			}

		} else if(piece_targets & changed_squares){ //last move changed squares in this pieces targets, generate moves for this piece
			idx = bit_to_idx(piece_loc);
			pl[to_move][i].pinned = 0;
			if(piece_type != PAWN){
				if(piece_type == ROOK)
					piece_control = MLUT.get_rook_attacks(idx, m);
				else if(piece_type == BISHOP)
					piece_control = MLUT.get_bishop_attacks(idx, m);
				else if(piece_type == QUEEN)
					piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
				else
					piece_control = MLUT.get_move_mask(piece_type, idx);
				piece_targets = piece_control;
			} else {
				u = MLUT.get_pawn_attack_mask(to_move,idx);
				piece_control = u;
				u |= create_pawn_pushes(piece_loc, to_move);
				piece_targets = u;
			}
		} else if(was_pinned){
			idx = bit_to_idx(piece_loc);
			if(piece_loc & unpins){
				if(piece_type != PAWN){
					if(piece_type == ROOK)
						piece_control = MLUT.get_rook_attacks(idx, m);
					else if(piece_type == BISHOP)
						piece_control = MLUT.get_bishop_attacks(idx, m);
					else if(piece_type == QUEEN)
						piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
					else
						piece_control = MLUT.get_move_mask(piece_type, idx);
					piece_targets = piece_control;
				} else {
					u = MLUT.get_pawn_attack_mask(to_move,idx);
					piece_control = u;
					u |= create_pawn_pushes(piece_loc, to_move);
					piece_targets = u;
				}
			} else if((piece_type != PAWN) && (piece_control & changed_squares)){
				if(piece_type == ROOK)
					piece_control = MLUT.get_rook_attacks(idx, m);
				else if(piece_type == BISHOP)
					piece_control = MLUT.get_bishop_attacks(idx, m);
				else if(piece_type == QUEEN)
					piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
				else
					piece_control = MLUT.get_move_mask(piece_type, idx);
			}
		}
		ally_ctrl |= piece_control;
		ally_targ |= piece_targets;
	}

	// Phase 4: Update move targets for enemy pieces affected by the last move or pin changes.
	// iterate through black non-king pieces,
	// generate moves if last move changed them or the piece is in a black king pin ray
	// OR if piece is flagged as pinned but not in pin ray
	// if the piece is not there its been promoted/castled, change accordingly.

	m = (occ[0] | occ[1]) & ~king_loc;
	for(i=1;i<num_pieces[!to_move];i++){
		U64& piece_loc = pl[!to_move][i].loc;
		piece_type = pl[!to_move][i].piece_type;
		if(piece_loc & last_move_to_and_from){ //piece was moved
			piece_loc = last_move_to_and_from & ~piece_loc;
			if((piece_type == PAWN) && (last_move_to_and_from & PROMOTION_RANKS)){
				piece_type = piece_at[bit_to_idx(piece_loc)];
				pl[!to_move][i].piece_type = piece_type;
			}
		}
		U64& piece_targets = pl[!to_move][i].targets;
		U64& piece_control = pl[!to_move][i].ctrl_sq;
		was_pinned = pl[!to_move][i].pinned;
		if(piece_loc & partial_pins[!to_move]){ //piece is on a partial pin ray that changed, generate moves for this piece
			idx = bit_to_idx(piece_loc);
			if(piece_type != PAWN){
				if(piece_type == ROOK)
					piece_control = MLUT.get_rook_attacks(idx, m);
				else if(piece_type == BISHOP)
					piece_control = MLUT.get_bishop_attacks(idx, m);
				else if(piece_type == QUEEN)
					piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
				else
					piece_control = MLUT.get_move_mask(piece_type, idx);
				if(piece_loc & actual_pins[!to_move]){
					pl[!to_move][i].pinned = 1;
					piece_targets = piece_control & pin_rays[!to_move][get_ray_dir(e_king_idx,idx)];
				} else {
					pl[!to_move][i].pinned = 0;
					piece_targets = piece_control;
				}
			} else {
				u = MLUT.get_pawn_attack_mask(!to_move,idx);
				piece_control = u;
				u |= create_pawn_pushes(piece_loc, !to_move);
				piece_targets = u;
				if(piece_loc & actual_pins[!to_move]){
					pl[!to_move][i].pinned = 1;
					piece_targets = piece_targets & pin_rays[!to_move][get_ray_dir(e_king_idx,idx)];
				} else {
					pl[!to_move][i].pinned = 0;
				}
			}

		} else if(piece_targets & changed_squares){ //last move changed squares in this pieces targets, generate moves for this piece
			idx = bit_to_idx(piece_loc);
			pl[!to_move][i].pinned = 0;
			if(piece_type != PAWN){
				if(piece_type == ROOK)
					piece_control = MLUT.get_rook_attacks(idx, m);
				else if(piece_type == BISHOP)
					piece_control = MLUT.get_bishop_attacks(idx, m);
				else if(piece_type == QUEEN)
					piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
				else
					piece_control = MLUT.get_move_mask(piece_type, idx);
				piece_targets = piece_control;
			} else {
				u = MLUT.get_pawn_attack_mask(!to_move,idx);
				piece_control = u;
				u |= create_pawn_pushes(piece_loc, !to_move);
				piece_targets = u;
			}
		} else if(was_pinned){
			idx = bit_to_idx(piece_loc);
			if(changed_squares & e_king_loc){ //piece is not pinned anymore , generate moves
				pl[!to_move][i].pinned = 0;
				if(piece_type != PAWN){
					if(piece_type == ROOK)
						piece_control = MLUT.get_rook_attacks(idx, m);
					else if(piece_type == BISHOP)
						piece_control = MLUT.get_bishop_attacks(idx, m);
					else if(piece_type == QUEEN)
						piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
					else
						piece_control = MLUT.get_move_mask(piece_type, idx);
					piece_targets = piece_control;
				} else {
					u = MLUT.get_pawn_attack_mask(!to_move,idx);
					piece_control = u;
					u |= create_pawn_pushes(piece_loc, !to_move);
					piece_targets = u;
				}
			} else if(piece_control & changed_squares){ //although piece is pinned control squares must be updated
				if(piece_type != PAWN){
					if(piece_type == ROOK)
						piece_control = MLUT.get_rook_attacks(idx, m);
					else if(piece_type == BISHOP)
						piece_control = MLUT.get_bishop_attacks(idx, m);
					else if(piece_type == QUEEN)
						piece_control = MLUT.get_rook_attacks(idx, m) | MLUT.get_bishop_attacks(idx, m);
					else
						piece_control = MLUT.get_move_mask(piece_type, idx);
				}
			}
		}
		enem_ctrl |= piece_control;
		enem_targ |= piece_targets;
	}



	// generate black king ctrl squares
	//

	u = MLUT.get_move_mask(KING,e_king_idx);
	enem_ctrl |= u;
	enem_targ |= u;



	//see if any pawns or knights are checking the white king. sliders were checked during the pin checks.

	u = MLUT.get_move_mask(KNIGHT,king_idx) & pieces[!to_move][KNIGHT];
	if(u){
		in_check++;
		king_attacker_ray = u;
	}
	u = MLUT.get_pawn_attack_mask(to_move,king_idx) & pieces[!to_move][PAWN];
	if(u){
		in_check++;
		king_attacker_ray = u;
		if(ep_sq){
			if(to_move){
				if(ep_sq << 8 == u) ep_check = true;
			} else {
				if(ep_sq >> 8 == u)	ep_check = true;
			}
		}
	}

	// Phase 5: Generate the legal move list from stored targets, applying check and pin filters.
	// load moves for white into list.
	// if in check only load king moves and moves on the check ray.
	// if double check stop after the king moves.

	// generate white king moves using compiled squares controlled by black
	u = MLUT.get_move_mask(KING,king_idx);
	ally_ctrl |= u;
	u &= ~(occ[to_move] | enem_ctrl);
	ally_targ |= u;
	src_square = king_idx << SRC_SHIFT;

	while(bitops::bscanf64( &idx, u)){
		u &= u-1;
		move = src_square + (idx << DST_SHIFT);
		mList.push_move(move);
	}

	v = occ[0] | occ[1];
	if(in_check == 0){
		for(i=1;i<num_pieces[to_move];i++){
			src_square = bit_to_idx(pl[to_move][i].loc) << SRC_SHIFT;
			if(pl[to_move][i].piece_type != PAWN){
				u = pl[to_move][i].targets;
				u &= ~occ[to_move];
				while(bitops::bscanf64( &idx, u)){
					u &= u-1;
					move = src_square + (idx << DST_SHIFT);
					mList.push_move(move);
				}
			} else {
				u = pl[to_move][i].targets;
				u &= ~(v & ~pl[to_move][i].ctrl_sq);
				u &= ~(pl[to_move][i].ctrl_sq & ~(occ[!to_move] | ep_sq));
				while(bitops::bscanf64( &idx, u)){
					u &= u-1;
					move = src_square + (idx << DST_SHIFT);
					if(ep_sq >> idx == 1){
						mList.push_move(move+ENPAS);
					} else if(((U64)1<<idx) & PROMOTION_RANKS){
						mList.push_promo_move(move);
					} else {
						mList.push_move(move);
					}
				}
			}
		}

		//CASTLING FOR SIDE TO MOVE
		if(to_move){
			can_castle = castlable_rooks & 0xFF00000000000000;
		} else {
			can_castle = castlable_rooks & 0x00000000000000FF;
		}
		while(bitops::bscanf64( &idx, can_castle)){
			can_castle &= can_castle-1;
			if(king_idx > idx){
				if(to_move){
					if(KSCB_CHECK & enem_ctrl) continue;
					if(KSCB_CLEAR & (occ[0] | occ[1])) continue;
					move = (king_idx << SRC_SHIFT) + (KSCB_KING_IDX << DST_SHIFT);
				} else {
					if(KSCW_CHECK & enem_ctrl) continue;
					if(KSCW_CLEAR & (occ[0] | occ[1])) continue;
					move = (king_idx << SRC_SHIFT) + (KSCW_KING_IDX << DST_SHIFT);
				}
			} else {
				//queenside
				if(to_move){
					if(QSCB_CHECK & enem_ctrl) continue;
					if(QSCB_CLEAR & (occ[0] | occ[1])) continue;
					move = (king_idx << SRC_SHIFT) + (QSCB_KING_IDX << DST_SHIFT);
				} else {
					if(QSCW_CHECK & enem_ctrl) continue;
					if(QSCW_CLEAR & (occ[0] | occ[1])) continue;
					move = (king_idx << SRC_SHIFT) + (QSCW_KING_IDX << DST_SHIFT);
				}
			}
			mList.push_move(move+CASTL);
		}
	} else if(in_check == 1){
		for(i=1;i<num_pieces[to_move];i++){
			src_square = bit_to_idx(pl[to_move][i].loc) << SRC_SHIFT;
			if(pl[to_move][i].piece_type != PAWN){
					u = pl[to_move][i].targets;
					u &= ~occ[to_move];
					u &= king_attacker_ray;
					while(bitops::bscanf64( &idx, u)){
						u &= u-1;
						move = src_square + (idx << DST_SHIFT);
						mList.push_move(move);
					}
			} else {
				u = pl[to_move][i].targets;
				u &= ~(v & ~pl[to_move][i].ctrl_sq);
				u &= ~(pl[to_move][i].ctrl_sq & ~(occ[!to_move] | ep_sq));
				if(ep_check){
					u &= king_attacker_ray | ep_sq;
				} else {
					u &= king_attacker_ray;
				}
				while(bitops::bscanf64( &idx, u)){
					u &= u-1;
					move = src_square + (idx << DST_SHIFT);
					if(ep_sq >> idx == 1){
						mList.push_move(move+ENPAS);
					} else if(((U64)1<<idx) & PROMOTION_RANKS){
						mList.push_promo_move(move);
					} else {
						mList.push_move(move);
					}
				}
			}
		}
	}

	ctrl[to_move] = ally_ctrl;
	ctrl[!to_move] = enem_ctrl;
	targ[to_move] = ally_targ;
	targ[!to_move] = enem_targ;
	captures = ally_targ & occ[!to_move];
	changed_squares = 0;
	last_move_to_and_from = 0;

	if(EXCLUDE_PAWNS_FROM_CAPTURE_MASK) captures &= ~pieces[!to_move][PAWN];

}

void chess_pos::order_moves()
{
	//rudimentary move ordering
	//put captures first in line to be popped
	int i,j;
	j = get_num_moves();
	for(i=j-1;i>=0;i--){
		if(((U64)1<<((mList.get_move(i) & DST_MASK) >> DST_SHIFT)) & captures){
			mList.swap_moves(i,(j--)-1);
		} 
	}
}

int chess_pos::order_moves_smart()
{
	chess_pos pos;
	int m_s[MAX_MOVES_IN_POS];
	int noreduce = 0;
	int i, color = 1-to_move*2;
	uint16_t move;
	int stand_pat = this->eval()*color, delta;
	for(i = get_num_moves() - 1;i>=0;i--){
		move = mList.get_move(i);
		pos.copy_pos(*this);
		pos.add_move(move);
		pos.generate_moves();
		delta = color*pos.eval() - stand_pat;
		delta = (delta+(pos.in_check*R_MAT));
		m_s[i] = delta;
		if(delta > (P_MAT*3)/2) noreduce++;
	}
	mList.sort_moves_by_scores(m_s);
	return noreduce;
}

int chess_pos::order_moves_mvvlva()
{
    // Score moves using MVV-LVA (Most Valuable Victim - Least Valuable Attacker).
    // Higher score = searched earlier (sort_moves_by_scores puts high scores at top of FILO stack).
    // Captures score above quiet moves; within captures, high-value victims and low-value attackers score higher.
    static const int mvv_lva[6] = {P_MAT, N_MAT, B_MAT, R_MAT, Q_MAT, 10000}; // P,N,B,R,Q,K
    int scores[MAX_MOVES_IN_POS];
    int num_moves = get_num_moves();
    int num_noreduce = 0;

    for(int i = 0; i < num_moves; i++){
        uint16_t move = mList.get_move(i);
        int dst_idx = (move & DST_MASK) >> DST_SHIFT;
        int src_idx = (move & SRC_MASK) >> SRC_SHIFT;
        U64 dst_sq = (U64)1 << dst_idx;
        int special = move & SPECIAL_MASK;

        if(dst_sq & occ[!to_move]){
            // Capture (including capture-promotions)
            int victim   = mvv_lva[piece_at[dst_idx]];
            int attacker = mvv_lva[piece_at[src_idx]];
            scores[i] = victim * 10 - attacker + 100000; // bias above quiet moves
            num_noreduce++;
        } else if(special == ENPAS){
            // En passant: pawn takes pawn
            scores[i] = P_MAT * 10 - P_MAT + 100000;
            num_noreduce++;
        } else if(special == PROMO){
            // Non-capture promotion (capture-promotions handled above)
            scores[i] = Q_MAT + 100000;
            num_noreduce++;
        } else {
            scores[i] = 0;
        }
    }

    mList.sort_moves_by_scores(scores);
    return num_noreduce;
}

U64 chess_pos::create_pawn_pushes(U64 pawn_loc, int side)
{
	U64 b = occ[0] | occ[1];

	if(side){
		// Black: push one square up (>>8). If on rank 7 and square ahead is empty, also offer double push.
		U64 single_push = pawn_loc >> 8;
		if((pawn_loc & RANK_7) && !(b & single_push)){
			return single_push | (pawn_loc >> 16); // single + double push
		}
		return single_push;
	} else {
		// White: push one square up (<<8). If on rank 2 and square ahead is empty, also offer double push.
		U64 single_push = pawn_loc << 8;
		if((pawn_loc & RANK_2) && !(b & single_push)){
			return single_push | (pawn_loc << 16); // single + double push
		}
		return single_push;
	}
}

int chess_pos::fwd_null_move(){
	if(next == NULL) return 0;
	if(in_check || bitops::popcount64(occ[to_move]) < 4 || last_move_null) return 0;
	next->copy_pos(*this);
	next->add_null_move();
	return 1;
}

void chess_pos::add_null_move(){
	zobrist_key ^= z_key(ep_target_square);
	zobrist_key ^= MLUT.get_zobrist_btm();
	to_move = !to_move; 
	last_move_null = 1;
	ep_target_square = 0;
	last_move_check_evasion = 0;
	changed_squares = 0;
}

uint16_t chess_pos::generate_and_add_random(){
	generate_moves();
	uint16_t move = mList.get_random_move();
	if(move) add_move(move);
	return move;
}

void chess_pos::add_move(uint16_t move)
{
	int src_idx = unsigned((move & SRC_MASK)>>SRC_SHIFT);
	int dst_idx = unsigned((move & DST_MASK)>>DST_SHIFT);
	U64 src_square = (U64)1 << src_idx; 
	U64 dst_square = (U64)1 << dst_idx;
	int piece_type = piece_at[src_idx];

	zobrist_key ^= z_key(ep_target_square);
	zobrist_key ^= z_key(castlable_rooks);
	zobrist_key ^= MLUT.get_zobrist_piece(to_move,piece_type,src_idx);
	zobrist_key ^= MLUT.get_zobrist_btm();

	if(dst_square & occ[!to_move]){
		last_move_capture = 1;
		zobrist_key ^= MLUT.get_zobrist_piece(!to_move,piece_at[dst_idx],dst_idx);
	} else {
		last_move_capture = 0;
	}

	last_move_null = 0;
	ep_target_square = 0;
	
	if((move & SPECIAL_MASK) == 0){
		zobrist_key ^= MLUT.get_zobrist_piece(to_move,piece_type,dst_idx);
		changed_squares = src_square | dst_square;
		last_move_to_and_from = changed_squares;
		occ[to_move] ^= (dst_square | src_square);
		occ[!to_move] &= ~dst_square;
		pieces[to_move][piece_type] ^= (dst_square | src_square);
		pieces[!to_move][piece_at[dst_idx]] &= ~(dst_square);
		piece_at[dst_idx] = piece_type;
		if(piece_type == PAWN){
			//if the pawn moved two spaces
			if(dst_square/src_square + src_square/dst_square == 65536){
				if(to_move){
					ep_target_square = src_square >> 8;
				} else {
					ep_target_square = src_square << 8;
				}
			}
		} else if(castlable_rooks > 0){
			if((piece_type == ROOK) && (src_square & castlable_rooks)){
				castlable_rooks &= ~src_square;
			} else if(piece_type == KING){
				if(to_move){
					castlable_rooks &= ~0xFF00000000000000;
				} else {
					castlable_rooks &= ~0x00000000000000FF;
				}
			}
		}
		if(dst_square & castlable_rooks){
			castlable_rooks &= ~dst_square;
		}
	} else if((move & SPECIAL_MASK) == PROMO){
		changed_squares = src_square | dst_square;
		last_move_to_and_from = changed_squares;
		occ[to_move] ^= (dst_square | src_square);
		occ[!to_move] &= ~dst_square;
		pieces[to_move][piece_type] ^= src_square;
		piece_type = (move & PROMO_MASK) + PROMO_TO_PIECE;
		zobrist_key ^= MLUT.get_zobrist_piece(to_move,piece_type,dst_idx);
		pieces[to_move][piece_type] ^= dst_square;
		pieces[!to_move][piece_at[dst_idx]] &= ~(dst_square);
		piece_at[dst_idx] = piece_type;
		if(dst_square & castlable_rooks){
			castlable_rooks &= ~dst_square;
		}
	} else if((move & SPECIAL_MASK) == ENPAS){
		zobrist_key ^= MLUT.get_zobrist_piece(to_move,piece_type,dst_idx);
		changed_squares = src_square | dst_square;
		last_move_to_and_from = changed_squares;
		occ[to_move] ^= (dst_square | src_square);
		piece_at[dst_idx] =PAWN;
		pieces[to_move][PAWN] ^= (dst_square | src_square);
		if(to_move){
			dst_square <<= 8;
			zobrist_key ^= MLUT.get_zobrist_piece(!to_move,PAWN,dst_idx+8); 
		} else {
			dst_square >>= 8;
			zobrist_key ^= MLUT.get_zobrist_piece(!to_move,PAWN,dst_idx-8); 
		}
		changed_squares |= dst_square;
		pieces[!to_move][PAWN] &= ~(dst_square);
		occ[!to_move] &= ~dst_square;
	} else if((move & SPECIAL_MASK) == CASTL){
		zobrist_key ^= MLUT.get_zobrist_piece(to_move,piece_type,dst_idx);
		if(src_square > dst_square){
			//king side
			if(to_move){
				dst_square = KSCB_CHANGED_SQUARES & castlable_rooks;
				changed_squares = KSCB_CHANGED_SQUARES;
				last_move_to_and_from = dst_square | KSCB_ROOK_SQUARE;
				pieces[to_move][ROOK] ^= (dst_square | KSCB_ROOK_SQUARE);
				pieces[to_move][KING] ^= (src_square | KSCB_KING_SQUARE);
				occ[to_move] ^= (KSCB_ROOK_SQUARE ^ KSCB_KING_SQUARE ^ dst_square ^ src_square);
				piece_at[KSCB_ROOK_IDX] = ROOK;
				piece_at[KSCB_KING_IDX] = KING;
				pl[BLACK][0].loc = KSCB_KING_SQUARE;
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,bit_to_idx(dst_square));
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,KSCB_ROOK_IDX);
			} else {
				dst_square = KSCW_CHANGED_SQUARES & castlable_rooks;
				changed_squares = KSCW_CHANGED_SQUARES;
				last_move_to_and_from = dst_square | KSCW_ROOK_SQUARE;
				pieces[to_move][ROOK] ^= (dst_square | KSCW_ROOK_SQUARE);
				pieces[to_move][KING] ^= (src_square | KSCW_KING_SQUARE);
				occ[to_move] ^= (KSCW_ROOK_SQUARE ^ KSCW_KING_SQUARE ^ dst_square ^ src_square);
				piece_at[KSCW_ROOK_IDX] = ROOK;
				piece_at[KSCW_KING_IDX] = KING;
				pl[WHITE][0].loc = KSCW_KING_SQUARE;
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,bit_to_idx(dst_square));
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,KSCW_ROOK_IDX);
			}
		} else {
			//queen side
			if(to_move){
				dst_square = QSCB_CHANGED_SQUARES & castlable_rooks;
				changed_squares = QSCB_CHANGED_SQUARES;
				last_move_to_and_from = dst_square | QSCB_ROOK_SQUARE;
				pieces[to_move][ROOK] ^= (dst_square | QSCB_ROOK_SQUARE);
				pieces[to_move][KING] ^= (src_square | QSCB_KING_SQUARE);
				occ[to_move] ^= (QSCB_ROOK_SQUARE ^ QSCB_KING_SQUARE ^ dst_square ^ src_square);
				piece_at[QSCB_ROOK_IDX] = ROOK;
				piece_at[QSCB_KING_IDX] = KING;
				pl[BLACK][0].loc = QSCB_KING_SQUARE;
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,bit_to_idx(dst_square));
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,QSCB_ROOK_IDX);
			} else {
				dst_square = QSCW_CHANGED_SQUARES & castlable_rooks;
				changed_squares = QSCW_CHANGED_SQUARES;
				last_move_to_and_from = dst_square | QSCW_ROOK_SQUARE;
				pieces[to_move][ROOK] ^= (dst_square | QSCW_ROOK_SQUARE);
				pieces[to_move][KING] ^= (src_square | QSCW_KING_SQUARE);
				occ[to_move] ^= (QSCW_ROOK_SQUARE ^ QSCW_KING_SQUARE ^ dst_square ^ src_square);
				piece_at[QSCW_ROOK_IDX] = ROOK;
				piece_at[QSCW_KING_IDX] = KING;
				pl[WHITE][0].loc = QSCW_KING_SQUARE;
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,bit_to_idx(dst_square));
				zobrist_key ^= MLUT.get_zobrist_piece(to_move,ROOK,QSCW_ROOK_IDX);
			}
		}
		if(to_move){
			castlable_rooks &= ~0xFF00000000000000;
		} else {
			castlable_rooks &= ~0x00000000000000FF;
		}
	}

	last_move_check_evasion = in_check;
	to_move = !to_move;

	zobrist_key ^= z_key(ep_target_square);
	zobrist_key ^= z_key(castlable_rooks);
}

uint16_t chess_pos::pop_and_add()
{
	uint16_t move;
	if(next == NULL) return 0;
	move = mList.pop_move();
	if(move == 0) return 0;
	add_move_to_next_node(move);
	return move;
}

uint16_t chess_pos::pop_and_add_capture()
{
	uint16_t move;
	if(next == NULL) return 0;
	move = mList.pop_targeted_move(captures);
	if(move > 1) add_move_to_next_node(move);
	return move;
}

int chess_pos::clear_next_occs()
{
	chess_pos* curr_pos = this;
	int total_depth = 0;
	while(curr_pos->next != nullptr){
		curr_pos = curr_pos->next;
		if (curr_pos->occ[WHITE] == 0
			|| curr_pos->occ[BLACK] == 0){
			break;
		}
		curr_pos->occ[WHITE] = curr_pos->occ[BLACK] = 0;
		total_depth++;
	}
	return total_depth;
}

void chess_pos::print_pos(bool basic)
{
	int row,file,k,p;
	char a;
	char piece_chars[2][6] = {{'P','N','B','R','Q','K'},{'p','n','b','r','q','k'}};
	string o = "\n";

	for(row=0 ; row < 8 ; row++){
		for(file =0; file <8; file++){
			o += " ";
			for(k=0;k<2;k++){
				p = piece_at_idx(63-(8*row+file),k);
				if(p >= 0){
					o += piece_chars[k][p];
					break;
				}
			}
			if(p < 0) o += "-";
		}
		if(!basic){
			switch(row){
				case 0:
					o += "	to move: ";
					o += !to_move ? "white" : "black";
					break;
				case 1:
					o +=  "	en passant target: ";
					o += ep_target_square ? idx_to_coord(bit_to_idx(ep_target_square)) : "-";
					break;
				case 2:
					o += "	castling: ";
					if(castlable_rooks & KSCW_CHANGED_SQUARES) o += "K ";
					if(castlable_rooks & KSCB_CHANGED_SQUARES) o += "k ";
					if(castlable_rooks & QSCW_CHANGED_SQUARES) o += "Q ";
					if(castlable_rooks & QSCB_CHANGED_SQUARES) o += "q ";
					break;
				case 3:
					o += "	zobrist: " + to_string(zobrist_key);
					break;
				case 4:

					break;
			}
		}
		o += "\n";
	}

	o += '\n';

	cout << o << flush;
}

void chess_pos::dump_pos(ofstream& ofs) //for debugging
{
	uint32_t i,j;
	ofs << "POSDUMP:" << endl;

	for(i=0;i<=1;i++){
		ofs << "pl" << endl;
		for(j=0;j<bitops::popcount64(occ[i]);j++){
			ofs << piece_itos(pl[i][j].piece_type) << endl;
			//ofs << pl[i][j].pinned;
			//print_bitboard(pl[i][j].loc, ofs);
			//print_bitboard(pl[i][j].ctrl_sq, ofs);
			//print_bitboard(pl[i][j].targets, ofs);
		}
		ofs << "pr" << endl;
		for(j=0;j<8;j++){
			//print_bitboard(pin_rays[i][j], ofs);
		}
		ofs << "pieces" << endl;
		ofs << endl;
		for(j=0;j<6;j++){
			ofs << pieces[i][j];
		}
		ofs << "occ" << occ[i];
		ofs << "ctrl" << ctrl[i];
		ofs << "targ" << targ[i];
	}
	ofs << " capt" << captures;
	ofs << " lmce" << last_move_check_evasion;
	ofs << " lmc" << last_move_capture;
	ofs << " chsq" << changed_squares;
	ofs << " lmtf" << last_move_to_and_from;
	for(i=0;i<get_num_moves();i++){
		ofs << "	" << move_itos(mList.get_move(i));
	}
	ofs << endl;
}

void chess_pos::print_line()
{
	chess_pos* a,b;
	int depth = 0;
	string os = "";
	a = this;
	while(a->prev != nullptr){
		a = a->prev;
		depth++;
	}
	while(depth-- > 0){
		os += move_itos(*a->next - *a) + " ";
		a = a->next;
	}
	cout << os << endl;
}

string chess_pos::get_fen()
{
	//"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
	int i,n;
	string ofen = "", castling = "";
	n=0;
	for(i=63;i>=0;i--){
		if(occ[WHITE] & ((U64)1<<i)){
			if(n>0) ofen += to_string(n);
			n=0;
			ofen += "PNBRQK"[piece_at[i]];
		} else if(occ[BLACK] & ((U64)1<<i)){
			if(n>0) ofen += to_string(n);
			n=0;
			ofen += "pnbrqk"[piece_at[i]];
		} else {
			n++;
			if(i%8==0){
				ofen += to_string(n);
				n=0;
			}
		}
		if(i%8==0 && i>0) ofen += "/";
	}
	ofen += " ";
	ofen += "wb"[to_move];
	ofen += " ";
	if(castlable_rooks & KSCW_CHANGED_SQUARES) castling += "K";
	if(castlable_rooks & QSCW_CHANGED_SQUARES) castling += "Q";
	if(castlable_rooks & KSCB_CHANGED_SQUARES) castling += "k";
	if(castlable_rooks & QSCB_CHANGED_SQUARES) castling += "q";
	if(castling.size() < 1) castling = "-";
	ofen += castling + " ";
	if(ep_target_square){
		ofen += idx_to_coord(bit_to_idx(ep_target_square));
	} else {
		ofen += "-";
	}
	return ofen;
}

int chess_pos::is_material_draw()
{
	int n;
	
	// KvK, KBvK, KNvK, KdarkBvKlightB

	n = bitops::popcount64(occ[WHITE]+occ[BLACK]);

	if(n <= 2){
		return 1;
	} else if(n <= 3){
		if(bitops::popcount64(pieces[WHITE][KNIGHT]) == 1) return 1;
		if(bitops::popcount64(pieces[BLACK][KNIGHT]) == 1) return 1;
		if(bitops::popcount64(pieces[WHITE][BISHOP]) == 1) return 1;
		if(bitops::popcount64(pieces[BLACK][BISHOP]) == 1) return 1;
		return 0;
	} else if(n <= 4){
		if(bitops::popcount64(pieces[WHITE][BISHOP]) == 1) n--;
		if(bitops::popcount64(pieces[BLACK][BISHOP]) == 1) n--;
		if(n == 2){
			if(bitops::popcount64((pieces[BLACK][BISHOP] + pieces[WHITE][BISHOP]) & LIGHT_SQUARES) == 1){
				return 1;
			} 
		}
	}

	return 0;
}

int chess_pos::is_quiet()
{
	if(captures || (in_check > 0)){
		return 0;
	} 
	return 1;
}

int chess_pos::mate_eval()
{
	if(in_check > 0){
		if(to_move){
			return (CHECKMATE-this->ply);
		} else {
			return -(CHECKMATE-this->ply);
		}
	} else {
		return STALEMATE;
	}
}

unsigned int chess_pos::get_num_moves()
{
	return mList.get_num_moves();
}
 

int chess_pos::eval()
{
	int eval = 0;
	int material_sum, material_diff;
	int p,n,b,r,q,P,N,B,R,Q;
	int i,j,white,black,wking,bking;
	int woffense, boffense;
	static const int targ_mult[KING] = {1,4,5,4,2};

	if(get_num_moves()<=0){
		return mate_eval();
	}

	p = int(bitops::popcount64(pieces[BLACK][PAWN]));
	n = int(bitops::popcount64(pieces[BLACK][KNIGHT]));
	b = int(bitops::popcount64(pieces[BLACK][BISHOP]));
	r = int(bitops::popcount64(pieces[BLACK][ROOK]));
	q = int(bitops::popcount64(pieces[BLACK][QUEEN]));
	P = int(bitops::popcount64(pieces[WHITE][PAWN]));
	N = int(bitops::popcount64(pieces[WHITE][KNIGHT]));
	B = int(bitops::popcount64(pieces[WHITE][BISHOP]));
	R = int(bitops::popcount64(pieces[WHITE][ROOK]));
	Q = int(bitops::popcount64(pieces[WHITE][QUEEN]));

	white =  int(bitops::popcount64(occ[WHITE]));
	black =  int(bitops::popcount64(occ[BLACK]));
	wking = bit_to_idx(pieces[WHITE][KING]);
	bking = bit_to_idx(pieces[BLACK][KING]);

	material_sum = P_MAT*(p+P)+N_MAT*(n+N)+B_MAT*(b+B)+R_MAT*(r+R)+Q_MAT*(q+Q);
	material_diff = P_MAT*(-p+P)+N_MAT*(-n+N)+B_MAT*(-b+B)+R_MAT*(-r+R)+Q_MAT*(-q+Q);
	eval += material_diff;

	if(abs(material_diff) >= 5*P_MAT && material_sum <= 2*Q_MAT){

		eval -= 2*int(bitops::popcount64(flood_fill_king(pieces[BLACK][KING],ctrl[WHITE]|occ[BLACK]|occ[WHITE],6)));
		eval += 2*int(bitops::popcount64(flood_fill_king(pieces[WHITE][KING],ctrl[BLACK]|occ[WHITE]|occ[BLACK],6)));
		eval += -material_diff/(P_MAT)*board_dist(wking,bking);

	} else {

		eval += 2*((int(bitops::popcount64(ctrl[WHITE] & CENTER)) - int(bitops::popcount64(ctrl[BLACK] & CENTER))));

		eval += 4*int(bitops::popcount64(flood_fill_king(pieces[BLACK][KING],occ[BLACK]|occ[WHITE],3) & ctrl[WHITE]));
		eval -= 4*int(bitops::popcount64(flood_fill_king(pieces[WHITE][KING],occ[BLACK]|occ[WHITE],3) & ctrl[BLACK]));
		eval += 2*int(bitops::popcount64(flood_fill_king(pieces[WHITE][KING],occ[BLACK]|ctrl[BLACK],2) & occ[WHITE]));
		eval -= 2*int(bitops::popcount64(flood_fill_king(pieces[BLACK][KING],occ[WHITE]|ctrl[WHITE],2) & occ[BLACK]));

		woffense = int(bitops::popcount64(targ[WHITE]&occ[BLACK])) - int(bitops::popcount64(targ[BLACK]&occ[BLACK]));
		boffense = int(bitops::popcount64(targ[BLACK]&occ[WHITE])) - int(bitops::popcount64(targ[WHITE]&occ[WHITE]));

		eval += (woffense-boffense);

		for(i=1;i<white;i++){
			j = pl[WHITE][i].piece_type;
			if(j == PAWN) continue;
			eval += int(bitops::popcount64(pl[WHITE][i].targets))*targ_mult[j];
		}
		for(i=1;i<black;i++){
			j = pl[BLACK][i].piece_type;
			if(j == PAWN) continue;
			eval -= int(bitops::popcount64(pl[BLACK][i].targets))*targ_mult[j];
		}

	}

	eval += ((3*P_MAT)/2)*(int(bitops::popcount64(pieces[WHITE][PAWN] & (RANK_8 + RANK_7 + RANK_6))) - int(bitops::popcount64(pieces[BLACK][PAWN] & (RANK_1 + RANK_2 + RANK_3))));

	return EVAL_GRAIN*(eval/EVAL_GRAIN);
}