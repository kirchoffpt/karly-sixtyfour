#include "chess_pos.h"

#define U64 unsigned long long int

using namespace std;

chess_mask_LUT MLUT; //GLOBAL LUT

chess_pos::chess_pos(){
	int i,j,k;

	for(i=0;i<2;i++){
		for(j=0;j<6;j++){
			pieces[i][j] = 0;
		}
	}
	for(i=0;i<64;i++){
		piece_at[i] = 0;
	}
	for(i=0;i<8;i++){
		pin_rays[0][i] = 0;
		pin_rays[1][i] = 0;
	}
	for(i=0;i<MAX_PIECES_PER_SIDE;i++){
		for(j=0;j<2;j++){
			pl[j][i].piece_type = 0;
			pl[j][i].pinned = 0;
			pl[j][i].loc = 0;
			pl[j][i].ctrl_sq = 0;
			pl[j][i].targets = 0;
		}
	}
	occ[WHITE] = 0;
	occ[BLACK] = 0;
	ep_target_square = 0;
	castlable_rooks = 0; 
	to_move = 0;
	last_move_check_evasion = 0;
	last_move_capture = 0;
	changed_squares = 0;
	last_move_to_and_from = 0;
	zobrist_key = 0;

	return;
}

unsigned short chess_pos::operator - (chess_pos const &c1){
	unsigned short move; 
	chess_pos p1, p2;
	p1 = *const_cast<chess_pos*>(&c1);
	p1.next = &p2;
	p1.generate_moves();
	while(move = p1.pop_and_add()){
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

void chess_pos::copy_pos(chess_pos* source_pos){
	int i,j,k;
	U64 u;
	unsigned long idx;

	memcpy(pl, source_pos->pl, sizeof(pl)+sizeof(pin_rays));
	memcpy(piece_at, source_pos->piece_at, sizeof(piece_at));
	memcpy(pieces, source_pos->pieces, sizeof(piece_at));
	//memcpy(pin_rays, source_pos->pin_rays, sizeof(pin_rays));

	// //when optimized, almost as fast as memcpy
	// for(i=0;i<6;i++){
	// 	for(j=0;j<2;j++){
	// 		pieces[j][i] = source_pos->pieces[j][i];
	// 	}
	// 	u = pieces[WHITE][i] | pieces[BLACK][i];
	// 	while(_BitScanForward64(&idx,u)){
	// 		u &= ~(U64(1) << idx);
	// 		piece_at[idx] = i; 
	// 	}
	// }
	// // for(i=0;i<64;i++){
	// // 	piece_at[i] = source_pos->piece_at[i];
	// // }

	occ[WHITE] = source_pos->occ[WHITE];
	occ[BLACK] = source_pos->occ[BLACK];
	zobrist_key = source_pos->zobrist_key;
	ep_target_square = source_pos->ep_target_square;
	castlable_rooks = source_pos->castlable_rooks; 
	to_move = source_pos->to_move;
	in_check = source_pos->in_check;
	changed_squares = 0;
	last_move_null = source_pos->last_move_null;

	if(DEBUG){ 
		assert(to_move == source_pos->to_move);
		assert(castlable_rooks == source_pos->castlable_rooks);
		assert(ep_target_square == source_pos->ep_target_square);
		assert(in_check == source_pos->in_check);
	}

	return;
}

void chess_pos::sort_piece_list()
{
	int i,j,side,min,min_idx,p_type;
	U64 min_loc;
	piece_list_struct pl_temp;
	for(side=WHITE;side<=BLACK;side++){
		for(j=__popcnt64(occ[side])-1;j>0;j--){
			min = KING;
			for(i=j;i>0;i--){
				p_type = pl[side][i].piece_type;
				if(p_type < min){
					min = p_type;
					min_loc = pl[side][i].loc;
					min_idx = i;
				} else if(p_type == min){
					if(pl[side][i].loc < min_loc && side){
						min_loc = pl[side][i].loc;
						min_idx = i;
					} else if(pl[side][i].loc > min_loc && !side){
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
	return;
}


void chess_pos::load_new_fen(string FEN)
{
	int i,j,k,n;
	char piece_chars[2][6] = {{'P','N','B','R','Q','K'},{'p','n','b','r','q','k'}};
	char c_temp;
	unsigned long idx;
	U64 u;

	string position = FEN;

	next = nullptr;

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
			while(_BitScanForward64( &idx, u)){
				u ^= U64(1) << idx; 
				piece_at[idx] = i;
			}	
		}
	}

	// for(i=0;i<8;i++){
	// 	for(k=0;k<8;k++){
	// 		cout << piece_at[63-(i*8+k)] << " ";
	// 	}
	// 	cout << endl;
	// }

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
		ep_target_square = U64(1);
		ep_target_square <<= 7-(int(position[j])-97);
		j++;
		ep_target_square <<= 8*(atoi(&position[j])-1);
	}

	occ[0] = occ[1] = 0;
	for(i=0;i<6;i++){
		occ[0] |= pieces[0][i];
		occ[1] |= pieces[1][i];
	}
	if(DEBUG){
		assert(__popcnt64(occ[WHITE]) <= MAX_PIECES_PER_SIDE);
		assert(__popcnt64(occ[BLACK]) <= MAX_PIECES_PER_SIDE);
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

	return;
}

chess_pos::chess_pos(string FEN)
{
	load_new_fen(FEN);
	return;
}

void chess_pos::add_move_to_next_node(unsigned short move)
{

	next->copy_pos(this);

	next->add_move(move);

	return;
}

int chess_pos::piece_at_idx(int idx, int side)
{
	int i;

	for(i=0;i<6;i++){
		if( ((pieces[side][i] >> idx) & ll(1)) == 1 ) return i;
	}

	return -1;
}


U64 chess_pos::prune_blocked_moves(int piece_type, int move_mask_center_idx, U64* move_mask, U64 blockers)
{

	U64 a,u,actual_blockers = 0;
	unsigned long idx;
	bool reverse = false;

	u = (*move_mask & blockers);

	if(u > 0){
		if(piece_type == ROOK){
			//ROOK
			if (_BitScanReverse64( &idx, u & MLUT.get_straight_ray(0,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(0,idx);
			}
			if (_BitScanReverse64( &idx, u & MLUT.get_straight_ray(1,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(1,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_straight_ray(2,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(2,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_straight_ray(3,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(3,idx);
			} 
		} else if(piece_type == BISHOP){
			//BISHOP
			if (_BitScanReverse64( &idx, u & MLUT.get_diag_ray(0,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(0,idx);
			}
			if (_BitScanReverse64( &idx, u & MLUT.get_diag_ray(1,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(1,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_diag_ray(2,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(2,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_diag_ray(3,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(3,idx);
			} 
		} else if(piece_type == QUEEN){
			//ROOK
			if (_BitScanReverse64( &idx, u & MLUT.get_straight_ray(0,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(0,idx);
			}
			if (_BitScanReverse64( &idx, u & MLUT.get_straight_ray(1,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(1,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_straight_ray(2,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(2,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_straight_ray(3,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_straight_ray(3,idx);
			}
			//BISHOP
			if (_BitScanReverse64( &idx, u & MLUT.get_diag_ray(0,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(0,idx);
			}
			if (_BitScanReverse64( &idx, u & MLUT.get_diag_ray(1,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(1,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_diag_ray(2,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
				*move_mask &= ~MLUT.get_diag_ray(2,idx);
			} 
			if (_BitScanForward64( &idx, u & MLUT.get_diag_ray(3,move_mask_center_idx))){
				actual_blockers |= U64(1) << idx;
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
	int i, side;
	int p_type, idx;

	zobrist_key = 0;

	for(side=WHITE;side<=BLACK;side++){
		for(i=0;i<__popcnt64(occ[side]);i++){
			p_type = pl[side][i].piece_type;
			idx = bit_to_idx(pl[side][i].loc);
			zobrist_key ^= MLUT.get_zobrist_piece(side, p_type, idx);
		}
	}
	zobrist_key ^= z_key(ep_target_square);
	zobrist_key ^= z_key(castlable_rooks);
	if(to_move) zobrist_key ^= MLUT.get_zobrist_btm();

	return;
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
	int i, num_pieces;
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

		while(_BitScanForward64( &idx, attackers ^ king_loc)){
			curr_piece++;
			attacker_loc = ll(1)<<idx;
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

		//many loops are unfurled to avoid branches, these are unfurled because i'm lazy
		//ROOK
		u = straight_blockers;

		m = MLUT.get_straight_ray(0,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(0,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][E_DIR] = m;
		m = MLUT.get_straight_ray(1,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(1,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][S_DIR] = m; 
		m = MLUT.get_straight_ray(2,king_idx);
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(2,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][W_DIR] = m; 
		m = MLUT.get_straight_ray(3,king_idx);
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(3,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][N_DIR] = m;
		//BISHOP

		u = diag_blockers;

		m = MLUT.get_diag_ray(0,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(0,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][SE_DIR] = m;
		m = MLUT.get_diag_ray(1,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(1,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][SW_DIR] = m;
		m = MLUT.get_diag_ray(2,king_idx); 
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(2,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][NW_DIR] = m;
		m = MLUT.get_diag_ray(3,king_idx); 
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(3,idx);
			m |= (U64(1)<<idx);
		}
		pin_rays[side][NE_DIR] = m; 

	}
	// for(side = WHITE;side<=BLACK;side++){
	// 	for(i=0;i<8;i++){
	// 		cout << "PIN!" << endl;
	// 		print_bitboard(pin_rays[side][i]);
	// 		Sleep(2000);
	// 	}
	// }
	// for(side = WHITE;side<=BLACK;side++){
	// 	num_pieces = __popcnt64(occ[side]);
	// 	for(i=0;i<num_pieces;i++){
	// 		cout << piece_itos(pl[side][i].piece_type) << endl;
	// 		print_bitboard(pl[side][i].targets);
	// 		Sleep(1000);
	// 	}
	// }
}

void chess_pos::store_init_targets(U64 piece_loc, U64 targets, int pinned)
{
	//does not need to be fast
	int i,side, num_pieces;
	targets &= ~piece_loc;
	for(side = WHITE;side<=BLACK;side++){
		num_pieces = __popcnt64(occ[side]);
		for(i=0;i<num_pieces;i++){
			if(pl[side][i].loc == piece_loc){
				pl[side][i].targets = targets;
				pl[side][i].pinned = 0;
				if(pinned) pl[side][i].pinned = 1;
				return;
			}
		}
	}
	return;
}

void chess_pos::init_targets(int side)
{
	//this is essentially the old move generation code modified to store moves in the piece list struct
	//speed is not of concern here
	U64 enemies = occ[!side]; //enemies gets modified
	U64 allies = occ[side];
	U64 allied_pawns = pieces[side][PAWN];
	unsigned long idx, idx2, pinned_idx;
	int attacking_piece, pinned_piece;
	U64 u, m, u_temp, v, ab_ray, attacker_loc, king_loc, possible_pins, king_attacker_ray, e_controlled_sq;
	U64 ep_target_square_copy = ep_target_square;
	U64 pinned_slider_moves[8][2];
	int pinned_sliders = 0;
	U64 pinned_pawn_moves[8][2];
	int pinned_pawns = 0;
	unsigned short move, src_square;
	bool promotion = false;
	U64 can_castle;


	in_check = 0;
	king_loc = pieces[side][KING];

	e_controlled_sq = 0;

	pos_move_list.reset_num_moves();	

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
					if(_BitScanReverse64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(0,idx);
						if(__popcnt64(u_temp & (occ[0] | occ[1])) == 3){
							ep_target_square = 0;
						}
					}
				} else {
					//king right of en passant attackers
					u_temp = MLUT.get_straight_ray(2,bit_to_idx(king_loc));
					if(_BitScanForward64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(2,idx);
						if(__popcnt64(u_temp & (occ[0] | occ[1])) == 3){
							ep_target_square = 0;
						}
					}
				}
			}
		}
	}

	//find squares attacked by enemy and allied pieces that are pinned
	while(_BitScanForward64( &idx, enemies)){
		attacker_loc = ll(1)<<idx;
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
				possible_pins = ab_ray & occ[side] ^ king_loc;
				if(possible_pins == 0){
					//if no possible pinned pieces the king is attacked
					in_check++;
					king_attacker_ray = ab_ray;
				} else if( (__popcnt64(possible_pins) == 1) ){
					//check for obstructions, otherwise a piece is pinned
					//cout << "The " << piece_itos(this->piece_at_idx(bit_to_idx(possible_pins),side)) << " at " << idx_to_coord(bit_to_idx(possible_pins)) << " is pinned." << endl;
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
					} else if(TRUE/*pinned_piece != KNIGHT*/){
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
				king_attacker_ray = attacker_loc;
				if((attacking_piece == PAWN) && (ep_target_square)){
					if(ep_target_square/attacker_loc + attacker_loc/ep_target_square == 256) king_attacker_ray |= ep_target_square;
				}
			}
		}

		//prune enemy's moves blocked by different color pieces
		u |= u_temp;
		prune_blocked_moves(attacking_piece, idx, &u, occ[side] ^ king_loc);

		e_controlled_sq |=  u;
		u &= ~u_temp;
		//cout << "piece: " << piece_itos(attacking_piece) << endl;
		//print_bitboard(u);
	}

	//generate moves for side to move
	allies ^= king_loc;
	//PAWNS first
	while(_BitScanForward64( &idx, allied_pawns)){
		attacker_loc = ll(1)<<idx;
		allied_pawns ^= attacker_loc;

		u = create_pawn_pushes(attacker_loc,side);
		u |= MLUT.get_pawn_attack_mask(side,idx);

		store_init_targets(attacker_loc,u,FALSE);
		//cout << "piece: " << piece_itos(attacking_piece) << endl;
		//print_bitboard(u);
	}
	//pinned pawns
	while(pinned_pawns-- > 0){
		attacker_loc = pinned_pawn_moves[pinned_pawns][0];
		u = pinned_pawn_moves[pinned_pawns][1];
		store_init_targets(attacker_loc,u,TRUE);
	}

	king_attacker_ray &= ~ep_target_square;

	//other pieces next except for the king
	while(_BitScanForward64( &idx, allies)){
		attacker_loc = ll(1)<<idx;
		allies ^= attacker_loc;
		attacking_piece = piece_at[idx];

		u = MLUT.get_move_mask(attacking_piece,idx);		
		u_temp = prune_blocked_moves(attacking_piece, idx, &u, (occ[0] | occ[1]));
		u |= u_temp;

		store_init_targets(attacker_loc,u,FALSE);

		//cout << "piece: " << piece_itos(attacking_piece) << endl;
		//print_bitboard(u);
	}
	//pinned sliders
	while(pinned_sliders-- > 0){
		attacker_loc = pinned_slider_moves[pinned_sliders][0];
		u = pinned_slider_moves[pinned_sliders][1];
		store_init_targets(attacker_loc,u,TRUE);
	}

	//lastly the KING moves
	_BitScanForward64( &idx, king_loc);

	u = MLUT.get_move_mask(KING,idx);		
	u_temp = prune_blocked_moves(KING, idx, &u, (occ[0] | occ[1]));
	u |= u_temp;

	store_init_targets(king_loc,u,FALSE);

	ep_target_square = ep_target_square_copy;
}

void chess_pos::generate_moves()
{
	//note many fixed sized loops are unfurled to avoid branches
	//code here specifically is pretty nightmarish in an attempt to be faster
	U64 king_loc, e_king_loc, straight_blockers, diag_blockers, *p_loc, *p_targ, *p_ctrl;
	U64 u,m,v,u_temp,possible_pins, unpins=0;
	U64 actual_pins[2] = {0};
	U64 partial_pins[2] = {0}; //simply the cumulative of the pin rays that were added/changed from last move
	U64 num_pieces[2];
	U64 ally_ctrl = 0;
	U64 enem_ctrl = 0;
	U64 ally_targ = 0;
	U64 enem_targ = 0;
	U64 king_attacker_ray = 0xFFFFFFFFFFFFFFFF;
	U64 can_castle;
	U64 ep_sq = ep_target_square;
	int i,j, king_idx, e_king_idx, x,y,p_type,p_pinned,k;
	unsigned long idx;
	unsigned short move, src_square;
	bool ep_check = FALSE;
	U64 Move_Compare;

	pos_move_list.reset_num_moves();

	if(DEBUG >= 2){
		generate_moves_deprecated();
		Move_Compare = captures;
		pos_move_list.reset_num_moves();
	}

	in_check = 0;
	if(changed_squares == 0)changed_squares = occ[!to_move];

	num_pieces[to_move] = __popcnt64(occ[to_move]);
	num_pieces[!to_move] = __popcnt64(occ[!to_move]);

	//return;
	// load white king position. 
	// if squares changed in a pin ray, find the new pin ray for that direction
	// see which pieces are actually pinned on the rays and store loc in actual pins

	king_loc = pieces[to_move][KING];
	king_idx = bit_to_idx(king_loc);
	e_king_loc = pieces[!to_move][KING];
	e_king_idx = bit_to_idx(e_king_loc);

	//check for special case of en passant pin
	if(ep_sq > 0){
		if(king_loc & EN_PASSANT_ATTACKER_RANKS){
			u = MLUT.get_en_passant_attackers(bit_to_idx(ep_sq)) & pieces[to_move][PAWN];
			if(king_idx/8 == bit_to_idx(u)/8){
				m = (pieces[!to_move][ROOK]+pieces[!to_move][QUEEN]);
				if(king_loc > u){
					//king left of en passant attackers
					u_temp = MLUT.get_straight_ray(0,king_idx);
					if(_BitScanReverse64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(0,idx);
						if(__popcnt64(u_temp & (occ[0] | occ[1])) == 3){
							ep_sq = 0;
						}
					}
				} else {
					//king right of en passant attackers
					u_temp = MLUT.get_straight_ray(2,king_idx);
					if(_BitScanForward64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(2,idx);
						if(__popcnt64(u_temp & (occ[0] | occ[1])) == 3){
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

	if(changed_squares & pin_rays[to_move][E_DIR]){
		m = MLUT.get_straight_ray(0,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(0,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][E_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][E_DIR] = m;
	}
	if(changed_squares & pin_rays[to_move][S_DIR]){
		m = MLUT.get_straight_ray(1,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(1,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][S_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][S_DIR] = m;
	}
	if(changed_squares & pin_rays[to_move][W_DIR]){
		m = MLUT.get_straight_ray(2,king_idx);
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(2,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][W_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][W_DIR] = m;
	}
	if(changed_squares & pin_rays[to_move][N_DIR]){
		m = MLUT.get_straight_ray(3,king_idx);
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_straight_ray(3,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][N_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][N_DIR] = m;
	}
	//BISHOP

	u = diag_blockers;


	if(changed_squares & pin_rays[to_move][SE_DIR]){
		m = MLUT.get_diag_ray(0,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(0,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][SE_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][SE_DIR] = m;
	}
	if(changed_squares & pin_rays[to_move][SW_DIR]){
		m = MLUT.get_diag_ray(1,king_idx);
		if (_BitScanReverse64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(1,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][SW_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][SW_DIR] = m;
	}
	if(changed_squares & pin_rays[to_move][NW_DIR]){
		m = MLUT.get_diag_ray(2,king_idx);
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(2,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][NW_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][NW_DIR] = m;
	}
	if(changed_squares & pin_rays[to_move][NE_DIR]){
		m = MLUT.get_diag_ray(3,king_idx);
		if (_BitScanForward64( &idx, u & m)){
			m &= ~MLUT.get_diag_ray(3,idx);
			if((m & u_temp)==0){
				possible_pins = m & v;
				m |= (U64(1)<<idx);
				j = __popcnt64(possible_pins);
				if(j == 1){
					actual_pins[to_move] |= possible_pins;
				}else if( j == 0){
					in_check++;
					king_attacker_ray = m;
				} 
			} else {
				m |= (U64(1)<<idx);
			}
		}
		unpins |= (pin_rays[to_move][NE_DIR] & ~m);
		partial_pins[to_move] |= m;
		pin_rays[to_move][NE_DIR] = m;
	}


	// for(j = WHITE;j<=BLACK;j++){
	// 	for(i=0;i<8;i++){
	// 		print_bitboard(pin_rays[to_move][i]);
	// 		Sleep(20);
	// 	}
	// }

	// load black king position. 
	// save pin rays if last move moved to or from a pin ray. 
	// see which pieces are actually pinned on the rays.
	// if the last move was a king move, pin rays are regenerated in all directions.

	straight_blockers = pieces[to_move][QUEEN] | pieces[to_move][ROOK];
	diag_blockers = pieces[to_move][QUEEN] | pieces[to_move][BISHOP];

	v = occ[!to_move] & ~e_king_loc;

	u_temp = occ[to_move];

	if(e_king_loc & changed_squares){

			u = straight_blockers;	

			m = MLUT.get_straight_ray(0,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(0,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][E_DIR] = m;
			m = MLUT.get_straight_ray(1,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(1,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][S_DIR] = m;
			m = MLUT.get_straight_ray(2,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(2,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][W_DIR] = m;
			m = MLUT.get_straight_ray(3,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(3,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][N_DIR] = m;
		//BISHOP

		u = diag_blockers;

			m = MLUT.get_diag_ray(0,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(0,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][SE_DIR] = m;
			m = MLUT.get_diag_ray(1,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(1,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][SW_DIR] = m;
			m = MLUT.get_diag_ray(2,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(2,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][NW_DIR] = m;
			m = MLUT.get_diag_ray(3,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(3,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
				partial_pins[!to_move] |= m;
			}
			pin_rays[!to_move][NE_DIR] = m;


	} else {

		u = straight_blockers;
			
		if(changed_squares & pin_rays[!to_move][E_DIR]){
			m = MLUT.get_straight_ray(0,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(0,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
			}
			pin_rays[!to_move][E_DIR] = m;
			partial_pins[!to_move] |= m;
		}
		if(changed_squares & pin_rays[!to_move][S_DIR]){
			m = MLUT.get_straight_ray(1,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(1,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
			}
			pin_rays[!to_move][S_DIR] = m;
			partial_pins[!to_move] |= m;
		}
		if(changed_squares & pin_rays[!to_move][W_DIR]){
			m = MLUT.get_straight_ray(2,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(2,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins; 
				m |= (U64(1)<<idx);
			}
			partial_pins[!to_move] |= m;
			pin_rays[!to_move][W_DIR] = m;
		}
		if(changed_squares & pin_rays[!to_move][N_DIR]){
			m = MLUT.get_straight_ray(3,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_straight_ray(3,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins; 
				m |= (U64(1)<<idx);
			}
			partial_pins[!to_move] |= m;
			pin_rays[!to_move][N_DIR] = m;
		}
		//BISHOP

		u = diag_blockers;

		if(changed_squares & pin_rays[!to_move][SE_DIR]){
			m = MLUT.get_diag_ray(0,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(0,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
			}
			partial_pins[!to_move] |= m;
			pin_rays[!to_move][SE_DIR] = m;
		}
		if(changed_squares & pin_rays[!to_move][SW_DIR]){
			m = MLUT.get_diag_ray(1,e_king_idx);
			if (_BitScanReverse64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(1,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
			}
			partial_pins[!to_move] |= m;
			pin_rays[!to_move][SW_DIR] = m;
		}
		if(changed_squares & pin_rays[!to_move][NW_DIR]){
			m = MLUT.get_diag_ray(2,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(2,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins; 
				m |= (U64(1)<<idx);
			}
			partial_pins[!to_move] |= m;
			pin_rays[!to_move][NW_DIR] = m;
		}
		if(changed_squares & pin_rays[!to_move][NE_DIR]){
			m = MLUT.get_diag_ray(3,e_king_idx);
			if (_BitScanForward64( &idx, u & m)){
				m &= ~MLUT.get_diag_ray(3,idx);
				possible_pins = m & v;
				if(((m&u_temp)==0) && (__popcnt64(possible_pins) == 1)) actual_pins[!to_move] |= possible_pins;
				m |= (U64(1)<<idx);
			}
			partial_pins[!to_move] |= m;
			pin_rays[!to_move][NE_DIR] = m;
		}


	}


	// iterate through white non-king pieces, 
	// generate moves if last move changed them or the piece is in a new white king pin ray
	// if theres an enpassant target, generate moves of en passant attackers.
	// if the piece is not there its been captured. remove it from the list.

	// print_bitboard(actual_pins[to_move]);
	// print_bitboard(actual_pins[!to_move]);
	// print_pos(true);
	// Sleep(100);

	m = (occ[0] | occ[1]) & ~e_king_loc;
	for(i=1;i<num_pieces[to_move];i++){
		p_loc = &pl[to_move][i].loc;
		__assume(!(*p_loc & changed_squares));
		if(*p_loc & changed_squares){ //piece was captured
			pl[to_move][i] = pl[to_move][num_pieces[to_move]];
			*p_loc = pl[to_move][i].loc;
		}
		p_type = pl[to_move][i].piece_type;
		p_targ = &pl[to_move][i].targets;
		p_ctrl = &pl[to_move][i].ctrl_sq;
		p_pinned = pl[to_move][i].pinned;
		idx = bit_to_idx(*p_loc);
		if(*p_loc & partial_pins[to_move]){ //piece is on a partial pin ray that changed, generate moves for this piece
			if(p_type != PAWN){
				u = MLUT.get_move_mask(p_type,idx);
				u_temp = prune_blocked_moves(p_type, idx, &u, m);
				*p_ctrl = u | u_temp;
				if(*p_loc & actual_pins[to_move]){
					pl[to_move][i].pinned = 1;
					*p_targ = *p_ctrl & pin_rays[to_move][get_ray_dir(king_idx,idx)];
				} else {
					pl[to_move][i].pinned = 0;
					*p_targ = *p_ctrl;
				}
			} else {
				u = MLUT.get_pawn_attack_mask(to_move,idx);
				*p_ctrl = u;
				u |= create_pawn_pushes(*p_loc, to_move);
				*p_targ = u;
				if(*p_loc & actual_pins[to_move]){
					pl[to_move][i].pinned = 1;
					*p_targ = *p_targ & pin_rays[to_move][get_ray_dir(king_idx,idx)];
				} else {
					pl[to_move][i].pinned = 0;
				}
			}

		} else if(*p_targ & changed_squares){ //last move changed squares in this pieces targets, generate moves for this piece
			pl[to_move][i].pinned = 0;
			if(p_type != PAWN){
				u = MLUT.get_move_mask(p_type,idx);
				u_temp = prune_blocked_moves(p_type, idx, &u, m);
				*p_ctrl = u | u_temp;
				*p_targ = *p_ctrl;
			} else {
				u = MLUT.get_pawn_attack_mask(to_move,idx);
				*p_ctrl = u;
				u |= create_pawn_pushes(*p_loc, to_move);
				*p_targ = u;
			}
		} else if(p_pinned){
			if(*p_loc & unpins){
				if(p_type != PAWN){
					u = MLUT.get_move_mask(p_type,idx);
					u_temp = prune_blocked_moves(p_type, idx, &u, m);
					*p_ctrl = u | u_temp;
					*p_targ = *p_ctrl;
				} else {
					u = MLUT.get_pawn_attack_mask(to_move,idx);
					*p_ctrl = u;
					u |= create_pawn_pushes(*p_loc, to_move);
					*p_targ = u;
				}
			} else if((p_type != PAWN) && (*p_ctrl & changed_squares)){
				u = MLUT.get_move_mask(p_type,idx);
				u_temp = prune_blocked_moves(p_type, idx, &u, m);
				*p_ctrl = u | u_temp;
			}
		}
		ally_ctrl |= *p_ctrl;
		ally_targ |= *p_targ;
	}
	//print_bitboard(ally_ctrl);

	// 	iterate through black non-king pieces, 
	// generate moves if last move changed them or the piece is in a black king pin ray 
	// OR if piece is flagged as pinned but not in pin ray
	// if the piece is not there its been promoted/castled, change accordingly.

	m = (occ[0] | occ[1]) & ~king_loc;
	for(i=1;i<num_pieces[!to_move];i++){
		p_loc = &pl[!to_move][i].loc;
		p_type = pl[!to_move][i].piece_type;
		__assume(!(*p_loc & last_move_to_and_from));
		if(*p_loc & last_move_to_and_from){ //piece was moved
			*p_loc = last_move_to_and_from & ~*p_loc;
			pl[!to_move][i].loc = *p_loc;
			if((p_type == PAWN) && (last_move_to_and_from & PROMOTION_RANKS)){
				p_type = piece_at[bit_to_idx(*p_loc)];
				pl[!to_move][i].piece_type = p_type;
			}
		}
		p_targ = &pl[!to_move][i].targets;
		p_ctrl = &pl[!to_move][i].ctrl_sq;
		p_pinned = pl[!to_move][i].pinned;
		idx = bit_to_idx(*p_loc);
		if(*p_loc & partial_pins[!to_move]){ //piece is on a partial pin ray that changed, generate moves for this piece
			if(p_type != PAWN){
				u = MLUT.get_move_mask(p_type,idx);
				u_temp = prune_blocked_moves(p_type, idx, &u, m);
				*p_ctrl = u | u_temp;
				if(*p_loc & actual_pins[!to_move]){
					pl[!to_move][i].pinned = 1;
					*p_targ = *p_ctrl & pin_rays[!to_move][get_ray_dir(e_king_idx,idx)];
				} else {
					pl[!to_move][i].pinned = 0;
					*p_targ = *p_ctrl;
				}
			} else {
				u = MLUT.get_pawn_attack_mask(!to_move,idx);
				*p_ctrl = u;
				u |= create_pawn_pushes(*p_loc, !to_move);
				*p_targ = u;
				if(*p_loc & actual_pins[!to_move]){
					pl[!to_move][i].pinned = 1;
					*p_targ = *p_targ & pin_rays[!to_move][get_ray_dir(e_king_idx,idx)];
				} else {
					pl[!to_move][i].pinned = 0;
				}
			}

		} else if(*p_targ & changed_squares){ //last move changed squares in this pieces targets, generate moves for this piece
			pl[!to_move][i].pinned = 0;
			if(p_type != PAWN){
				u = MLUT.get_move_mask(p_type,idx);
				u_temp = prune_blocked_moves(p_type, idx, &u, m);
				*p_ctrl = u | u_temp;
				*p_targ = *p_ctrl;
			} else {
				u = MLUT.get_pawn_attack_mask(!to_move,idx);
				*p_ctrl = u;
				u |= create_pawn_pushes(*p_loc, !to_move);
				*p_targ = u;
				// print_pos(true);
				// print_bitboard(p_targ | *p_loc);
			}
		} else if(p_pinned){
			if(changed_squares & e_king_loc){ //piece is not pinned anymore , generate moves
				pl[!to_move][i].pinned = 0;
				if(p_type != PAWN){
					u = MLUT.get_move_mask(p_type,idx);
					u_temp = prune_blocked_moves(p_type, idx, &u, m);
					*p_ctrl = u | u_temp;
					*p_targ = *p_ctrl;
				} else {
					u = MLUT.get_pawn_attack_mask(!to_move,idx);
					*p_ctrl = u;
					u |= create_pawn_pushes(*p_loc, !to_move);
					*p_targ = u;
				}
			} else if(*p_ctrl & changed_squares){ //although piece is pinned control squares must be updated
				if(p_type != PAWN){
					u = MLUT.get_move_mask(p_type,idx);
					u_temp = prune_blocked_moves(p_type, idx, &u, m);
					*p_ctrl = u | u_temp;
				}
			}
		}
		enem_ctrl |= *p_ctrl;
		enem_targ |= *p_targ;
	}



	// generate black king ctrl squares
	//

	u = MLUT.get_move_mask(KING,e_king_idx);
	enem_ctrl |= u;
	enem_targ |= u;

	//print_bitboard(enem_ctrl);

	// print_pos(true);
	// print_bitboard(changed_squares);
	// cout << "pieces updated: " << k << endl;
	// cout << "pins: " << __popcnt64(actual_pins[to_move]) << endl;
	// cout << "checks: " << in_check << endl;

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
				if(ep_sq << 8 == u) ep_check = TRUE;
			} else {
				if(ep_sq >> 8 == u)	ep_check = TRUE;
			}
		}
	}

	// 	load moves for white into list. 
	// if in check only load king moves and moves on the check ray. 
	// if double check stop after the king moves.

		// generate white king moves using compiled squares controlled by black
	//

	u = MLUT.get_move_mask(KING,king_idx);
	ally_ctrl |= u;
	u &= ~(occ[to_move] | enem_ctrl);
	ally_targ |= u;
	src_square = king_idx << SRC_SHIFT;
	//captures = u & occ[!to_move];

	while(_BitScanForward64( &idx, u)){
		u ^= U64(1)<<idx;
		move = src_square + (idx << DST_SHIFT);
		pos_move_list.push_move(move);
	}

	v = occ[0] | occ[1];
	if(in_check == 0){
		for(i=1;i<num_pieces[to_move];i++){
			src_square = bit_to_idx(pl[to_move][i].loc) << SRC_SHIFT;
			if(pl[to_move][i].piece_type != PAWN){
				u = pl[to_move][i].targets;
				u &= ~occ[to_move];
				//captures |= u & occ[!to_move];
				while(_BitScanForward64( &idx, u)){
					u ^= U64(1)<<idx;
					move = src_square + (idx << DST_SHIFT);
					pos_move_list.push_move(move);
				}
			} else {
				u = pl[to_move][i].targets;
				u &= ~(v & ~pl[to_move][i].ctrl_sq);
				u &= ~(pl[to_move][i].ctrl_sq & ~(occ[!to_move] | ep_sq));
				//captures |= occ[!to_move] & pl[to_move][i].ctrl_sq;
				while(_BitScanForward64( &idx, u)){
					u ^= U64(1)<<idx;
					move = src_square + (idx << DST_SHIFT);
					if(ep_sq >> idx == 1){
						pos_move_list.push_move(move+ENPAS);
					} else if((U64(1)<<idx) & PROMOTION_RANKS){
						pos_move_list.push_promo_move(move);
					} else {
						pos_move_list.push_move(move);
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
		while(_BitScanForward64( &idx, can_castle)){
			can_castle ^= ll(1)<<idx;
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
			pos_move_list.push_move(move+CASTL);
		}
	} else if(in_check == 1){
		//king_attacker_ray &= ~king_loc;
		// print_pos(true);
		// print_bitboard(king_attacker_ray);
		for(i=1;i<num_pieces[to_move];i++){
			src_square = bit_to_idx(pl[to_move][i].loc) << SRC_SHIFT;
			if(pl[to_move][i].piece_type != PAWN){
					u = pl[to_move][i].targets;
					u &= ~occ[to_move];
					u &= king_attacker_ray;
					//captures |= u & occ[!to_move];
					while(_BitScanForward64( &idx, u)){
						u ^= U64(1)<<idx;
						move = src_square + (idx << DST_SHIFT);
						pos_move_list.push_move(move);
					}
			} else {
				u = pl[to_move][i].targets;
				u &= ~(v & ~pl[to_move][i].ctrl_sq);
				u &= ~(pl[to_move][i].ctrl_sq & ~(occ[!to_move] | ep_sq));
				//captures |= occ[!to_move] & pl[to_move][i].ctrl_sq & king_attacker_ray;
				if(ep_check){
					u &= king_attacker_ray | ep_sq;
				} else {
					u &= king_attacker_ray;
				}
				while(_BitScanForward64( &idx, u)){
					u ^= U64(1)<<idx;
					move = src_square + (idx << DST_SHIFT);
					if(ep_sq >> idx == 1){
						pos_move_list.push_move(move+ENPAS);
					} else if((U64(1)<<idx) & PROMOTION_RANKS){
						pos_move_list.push_promo_move(move);
					} else {
						pos_move_list.push_move(move);
					}
				}
			}
		}
	}

	ctrl[to_move] = ally_ctrl;
	ctrl[!to_move] = enem_ctrl;
	target_squares[to_move] = ally_targ;
	target_squares[!to_move] = enem_targ;
	captures = ally_targ & occ[!to_move];
	changed_squares = 0;
	last_move_to_and_from = 0;

	if(EXCLUDE_PAWNS_FROM_CAPTURE_MASK) captures &= ~pieces[!to_move][PAWN];

	// print_bitboard(ally_ctrl);
	// print_bitboard(enem_ctrl);

	// print_pos(true);
	// u=0;
	// for(i=0;i<8;i++){
	// 	u |= pin_rays[to_move][i];
	// }
	// print_bitboard(u);
	// u=0;
	// for(i=0;i<8;i++){
	// 	u |= pin_rays[!to_move][i];
	// }
	// print_bitboard(u);
	//cout << pos_move_list.get_num_moves() << endl;
	// Sleep(200);

	if(DEBUG >= 2){
	if(Move_Compare != captures){
			print_pos(false);
			print_bitboard(Move_Compare);
			print_bitboard(captures);
			//print_bitboard(pin_rays[to_move][NE_DIR]);
			// for(i=0;i<get_num_moves();i++){
			// 	print_move(pos_move_list.get_move(i));
			// 	cout << endl;
			// }
			// pos_move_list.reset_num_moves();
			// generate_moves_deprecated();
			// cout << endl;
			// for(i=0;i<get_num_moves();i++){
			// 	print_move(pos_move_list.get_move(i));
			// 	cout << endl;
			// }
			for(j=1;j<num_pieces[to_move];j++){
				print_bitboard( pl[to_move][j].targets);
			}
			cin >> i;
		}
	}
	return;
}

void chess_pos::order_moves()
{
	//rudimentary move ordering
	//put captures first in line to be popped
	int i,j;
	j = get_num_moves();
	for(i=j-1;i>=0;i--){
		if((U64(1)<<((pos_move_list.get_move(i) & DST_MASK) >> DST_SHIFT)) & captures){
			pos_move_list.swap_moves(i,(j--)-1);
		} 
	}
	return;	
}

int chess_pos::order_moves_smart()
{
	static chess_pos pos;
	static int m_s[MAX_MOVES_IN_POS];
	int noreduce = 0;
	int i, color = 1-to_move*2;
	unsigned short move;
	int stand_pat = this->eval()*color, delta;
	for(i = get_num_moves() - 1;i>=0;i--){
		move = pos_move_list.get_move(i);
		pos.copy_pos(this);
		pos.add_move(move);
		pos.generate_moves();
		delta = color*pos.eval() - stand_pat;
		delta = (delta+(pos.in_check*R_MAT));
		m_s[i] = delta;
		if(delta > (P_MAT*3)/2) noreduce++;
	}
	pos_move_list.sort_moves_by_scores(m_s);
	return noreduce+1;	
}

U64 chess_pos::create_pawn_pushes(U64 pawn_loc, int side)
{
	U64 b = occ[0] | occ[1];

	if(side){
		if(!((b << 8) & pawn_loc)&&(pawn_loc & RANK_7)){
			return (pawn_loc>>16)*257;
		} else {
			return (pawn_loc>>8);
		}
	} else {
		if(!((b >> 8) & pawn_loc)&&(pawn_loc & RANK_2)){
			return pawn_loc*65792;
		} else {
			return pawn_loc*256;
		}
	}
	return 0;
}

void chess_pos::generate_moves_deprecated()
{
	U64 enemies = occ[!to_move]; //enemies gets modified
	U64 allies = occ[to_move];
	U64 allied_pawns = pieces[to_move][PAWN];
	unsigned long idx, idx2, pinned_idx;
	int attacking_piece, pinned_piece;
	U64 u, m, u_temp, v, ab_ray, attacker_loc, king_loc, possible_pins, king_attacker_ray, e_controlled_sq, a_controlled_sq;
	U64 ep_target_square_copy = ep_target_square;
	U64 pinned_slider_moves[8][2];
	int pinned_sliders = 0;
	U64 pinned_pawn_moves[8][2];
	int pinned_pawns = 0;
	unsigned short move, src_square;
	bool promotion = false;
	U64 can_castle;

	if(to_move){
		can_castle = castlable_rooks & 0xFF00000000000000;
	} else {
		can_castle = castlable_rooks & 0x00000000000000FF;
	}

	in_check = 0;
	king_loc = pieces[to_move][KING];

	e_controlled_sq = 0;
	a_controlled_sq = 0;

	pos_move_list.reset_num_moves();	

	allies &= ~allied_pawns;

	//check for special case of en passant pin
	if(ep_target_square > 0){
		if(king_loc & EN_PASSANT_ATTACKER_RANKS){
			u = MLUT.get_en_passant_attackers(bit_to_idx(ep_target_square)) & allied_pawns;
			if(bit_to_idx(king_loc)/8 == bit_to_idx(u)/8){
				m = (pieces[!to_move][ROOK]+pieces[!to_move][QUEEN]);
				if(king_loc > u){
					//king left of en passant attackers
					u_temp = MLUT.get_straight_ray(0,bit_to_idx(king_loc));
					if(_BitScanReverse64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(0,idx);
						if(__popcnt64(u_temp & (occ[0] | occ[1])) == 3){
							ep_target_square = 0;
						}
					}
				} else {
					//king right of en passant attackers
					u_temp = MLUT.get_straight_ray(2,bit_to_idx(king_loc));
					if(_BitScanForward64(&idx, m & u_temp)){
						u_temp &= ~MLUT.get_straight_ray(2,idx);
						if(__popcnt64(u_temp & (occ[0] | occ[1])) == 3){
							ep_target_square = 0;
						}
					}
				}
			}
		}
	}

	idx = bit_to_idx(king_loc);
	if(can_castle == 0){
		//prune enemies of unimportant pieces (pieces not xraying the king or its flight squares)
			//pawns
		enemies &= ~( pieces[!to_move][PAWN] & ~MLUT.get_pawn_area_of_influence(to_move,idx) );
			//rooks
		enemies &= ~( pieces[!to_move][ROOK] & ~MLUT.get_rook_area_of_influence(idx) );
	} else {
			//pawns
		if(to_move){
			u = PAWN_RANK_BLACK;
		} else {
			u = PAWN_RANK_WHITE;
		}
		enemies &= ~( pieces[!to_move][PAWN] & ~(MLUT.get_pawn_area_of_influence(to_move,idx) | u) );
	}

	//find squares attacked by enemy and allied pieces that are pinned
	while(_BitScanForward64( &idx, enemies)){
		attacker_loc = ll(1)<<idx;
		enemies ^= attacker_loc;
		attacking_piece = piece_at[idx];

		if (attacking_piece==PAWN){
			u = MLUT.get_pawn_attack_mask(!to_move,idx);
		} else {
			u = MLUT.get_move_mask(attacking_piece,idx);
		}

		//prune enemy's moves blocked by same color pieces
		u_temp = prune_blocked_moves(attacking_piece, idx, &u, occ[!to_move]);

		//if xraying check for pinned pieces
		if(is_sliding_piece(attacking_piece)){
			if((king_loc & u) > 0 ) {
				ab_ray = MLUT.get_sliding_ray( king_loc | attacker_loc ); 
				possible_pins = ab_ray & occ[to_move] ^ king_loc;
				if(possible_pins == 0){
					//if no possible pinned pieces the king is attacked
					in_check++;
					king_attacker_ray = ab_ray;
				} else if( (__popcnt64(possible_pins) == 1) ){
					//check for obstructions, otherwise a piece is pinned
					//cout << "The " << piece_itos(this->piece_at_idx(bit_to_idx(possible_pins),to_move)) << " at " << idx_to_coord(bit_to_idx(possible_pins)) << " is pinned." << endl;
					pinned_idx = bit_to_idx(possible_pins);
					pinned_piece = piece_at[pinned_idx];
					if(in_check == 0){
						//GENERATE PINNED PIECE MOVES
						if (pinned_piece==PAWN){
							allied_pawns &= ~possible_pins;
							v = (occ[0] | occ[1]) ^ possible_pins;
							if(to_move){
								v |= v >> 8;
							} else {
								v |= v << 8;
							}
							m = MLUT.get_pawn_attack_mask(to_move,pinned_idx) & (occ[!to_move] | ep_target_square);
							m |= (MLUT.get_move_mask(PAWN+B_PAWN*to_move,pinned_idx) & ~v);
							m &= ab_ray ^ king_loc;
							pinned_pawn_moves[pinned_pawns][0] = possible_pins;
							pinned_pawn_moves[pinned_pawns++][1] = m;
						} else if(pinned_piece != KNIGHT){
							//sliding piece. store moves later in case another check is found
							allies ^= possible_pins;
							m = MLUT.get_move_mask(pinned_piece,pinned_idx);
							m &= ab_ray ^ king_loc;
							pinned_slider_moves[pinned_sliders][0] = possible_pins;
							pinned_slider_moves[pinned_sliders++][1] = m;
						} else {
							allies &= ~possible_pins;
						}

					} else {
						if(pinned_piece == PAWN){
							allied_pawns &= ~possible_pins;
						} else {
							allies &= ~possible_pins;
						}
					}
				}
				

			}
		} else {
			if((king_loc & u) > 0 ) {
				in_check++;
				king_attacker_ray = attacker_loc;
				if((attacking_piece == PAWN) && (ep_target_square)){
					if(ep_target_square/attacker_loc + attacker_loc/ep_target_square == 256) king_attacker_ray |= ep_target_square;
				}
			}
		}

		//prune enemy's moves blocked by different color pieces
		u |= u_temp;
		prune_blocked_moves(attacking_piece, idx, &u, occ[to_move] ^ king_loc);

		e_controlled_sq |=  u;
		u &= ~u_temp;
		//cout << "piece: " << piece_itos(attacking_piece) << endl;
		//print_bitboard(u);
	}

	//generate moves for side to move
	allies ^= king_loc;
	if(in_check < 2){
		//PAWNS first
		while(_BitScanForward64( &idx, allied_pawns)){
			attacker_loc = ll(1)<<idx;
			allied_pawns ^= attacker_loc;

			promotion = false;
			u_temp = (occ[0] | occ[1]) ^ attacker_loc;
			if(to_move){
				u_temp |= u_temp >> 8;
			} else {
				u_temp |= u_temp << 8;
			}
			u = (MLUT.get_move_mask(PAWN+B_PAWN*to_move,idx) & ~u_temp);
			u |= (MLUT.get_pawn_attack_mask(to_move,idx) & (occ[!to_move] | ep_target_square));
			if(in_check > 0) u &= king_attacker_ray;
			if((u & PROMOTION_RANKS) > 0) promotion = true;

			//store moves in move list for this position
			src_square = idx << SRC_SHIFT;
			a_controlled_sq |= u;
			while(_BitScanForward64( &idx2, u)){
				u ^= ll(1)<<idx2;
				move = src_square + (idx2 << DST_SHIFT);
				if(promotion){
					move += PROMO;
					pos_move_list.push_move(move+N_PROMO);
					pos_move_list.push_move(move+B_PROMO);
					pos_move_list.push_move(move+R_PROMO);
					pos_move_list.push_move(move+Q_PROMO);
				} else if(ep_target_square && ((U64(1)<<idx2) == ep_target_square)){
					pos_move_list.push_move(move+ENPAS);
				} else {
					pos_move_list.push_move(move);
				}
			}
			//cout << "piece: " << piece_itos(attacking_piece) << endl;
			//print_bitboard(u);
		}
		//pinned pawns
		while(pinned_pawns-- > 0){
			src_square = bit_to_idx(pinned_pawn_moves[pinned_pawns][0]) << SRC_SHIFT;
			u = pinned_pawn_moves[pinned_pawns][1];
			if(in_check > 0) u &= king_attacker_ray;
			promotion = false;
			if((u & PROMOTION_RANKS) > 0) promotion = true;
			a_controlled_sq |= u;
			while(_BitScanForward64( &idx2, u)){
				u ^= ll(1)<<idx2;
				move = src_square + (idx2 << DST_SHIFT);
				if(promotion){
					move += PROMO;
					pos_move_list.push_move(move+N_PROMO);
					pos_move_list.push_move(move+B_PROMO);
					pos_move_list.push_move(move+R_PROMO);
					pos_move_list.push_move(move+Q_PROMO);
				} else if(ep_target_square && ((U64(1)<<idx2) == ep_target_square)){
					pos_move_list.push_move(move+ENPAS);
				} else {
					pos_move_list.push_move(move);
				}
			}
		}

		king_attacker_ray &= ~ep_target_square;

		//other pieces next except for the king
		while(_BitScanForward64( &idx, allies)){
			attacker_loc = ll(1)<<idx;
			allies ^= attacker_loc;
			attacking_piece = piece_at[idx];

			u = MLUT.get_move_mask(attacking_piece,idx);		
			u_temp = prune_blocked_moves(attacking_piece, idx, &u, (occ[0] | occ[1]));
			u |= (u_temp & occ[!to_move]);
			if(in_check > 0) u &= king_attacker_ray;


			//store moves in move list for this position
			src_square = idx << SRC_SHIFT;
			a_controlled_sq |= u;
			while(_BitScanForward64( &idx2, u)){
				u ^= ll(1)<<idx2;
				move = src_square + (idx2 << DST_SHIFT);
				pos_move_list.push_move(move);
			}

			//cout << "piece: " << piece_itos(attacking_piece) << endl;
			//print_bitboard(u);
		}
		//pinned sliders
		while(pinned_sliders-- > 0){
			src_square = bit_to_idx(pinned_slider_moves[pinned_sliders][0]) << SRC_SHIFT;
			u = pinned_slider_moves[pinned_sliders][1];
			if(in_check > 0) u &= king_attacker_ray;
			a_controlled_sq |= u;
			while(_BitScanForward64( &idx2, u)){
				u ^= ll(1)<<idx2;
				move = src_square + (idx2 << DST_SHIFT);
				pos_move_list.push_move(move);
			}
		}
	}

	//lastly the KING moves
	_BitScanForward64( &idx, king_loc);

	u = MLUT.get_move_mask(KING,idx);		
	u &= ~e_controlled_sq;
	u_temp = prune_blocked_moves(KING, idx, &u, (occ[0] | occ[1]));
	u |= (u_temp & occ[!to_move]);

	//store moves in move list for this position
	src_square = idx << SRC_SHIFT;
	a_controlled_sq |= u;
	while(_BitScanForward64( &idx2, u)){
		u ^= ll(1)<<idx2;
		move = src_square + (idx2 << DST_SHIFT);
		pos_move_list.push_move(move);
	}

	if(in_check == 0){
		while(_BitScanForward64( &idx2, can_castle)){
			can_castle ^= ll(1)<<idx2;
			if(idx > idx2){
				if(to_move){
					if(KSCB_CHECK & e_controlled_sq) continue;
					if(KSCB_CLEAR & (occ[0] | occ[1])) continue;
					move = src_square + (KSCB_KING_IDX << DST_SHIFT);
				} else {
					if(KSCW_CHECK & e_controlled_sq) continue;
					if(KSCW_CLEAR & (occ[0] | occ[1])) continue;
					move = src_square + (KSCW_KING_IDX << DST_SHIFT);
				}
			} else {
				//queenside
				if(to_move){
					if(QSCB_CHECK & e_controlled_sq) continue;
					if(QSCB_CLEAR & (occ[0] | occ[1])) continue;
					move = src_square + (QSCB_KING_IDX << DST_SHIFT);
				} else {
					if(QSCW_CHECK & e_controlled_sq) continue;
					if(QSCW_CLEAR & (occ[0] | occ[1])) continue;
					move = src_square + (QSCW_KING_IDX << DST_SHIFT);
				}
			}
			pos_move_list.push_move(move+CASTL);
		}
	}

	captures = (a_controlled_sq & occ[!to_move]);

	ep_target_square = ep_target_square_copy;

}

int chess_pos::fwd_null_move(){
	if(next == NULL) return 0;
	if(in_check || __popcnt64(occ[to_move]) < 4 || last_move_null) return 0;
		if(last_move_null) cout << "LMN" << endl;
	next->copy_pos(this);
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
	return;
}

void chess_pos::add_move(unsigned short move)
{
	int src_idx = unsigned((move & SRC_MASK)>>SRC_SHIFT);
	int dst_idx = unsigned((move & DST_MASK)>>DST_SHIFT);
	U64 src_square = U64(1) << src_idx; 
	U64 dst_square = U64(1) << dst_idx;
	int piece_type = piece_at[src_idx];
	//cout << idx_to_coord(src_idx) << " " << idx_to_coord(dst_idx);

	zobrist_key ^= z_key(ep_target_square);
	zobrist_key ^= z_key(castlable_rooks);
	zobrist_key ^= MLUT.get_zobrist_piece(to_move,piece_type,src_idx);
	zobrist_key ^= MLUT.get_zobrist_btm();

	if(DEBUG){
		assert(src_square != dst_square);
	}

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

	return;
}

unsigned short chess_pos::pop_and_add()
{
	unsigned short move;
	if(next == NULL) return 0;
	move = pos_move_list.pop_move();
	if(move == 0) return 0;
	add_move_to_next_node(move);
	return move;
}

unsigned short chess_pos::pop_and_add_capture()
{
	unsigned short move;
	if(next == NULL) return 0;
	move = pos_move_list.pop_targeted_move(captures);
	if(move > 1) add_move_to_next_node(move);
	return move;
}

void chess_pos::print_pos(bool basic)
{
	int i,j,k,p;
	char a;
	char castle_chars[4] = {'K','Q','k','q'};
	char piece_chars[2][6] = {{'P','N','B','R','Q','K'},{'p','n','b','r','q','k'}};

	cout << '\n';

	for(i=0 ; i < 8 ; i++){
		for(j =0; j <8; j++){
			for(k=0;k<2;k++){
				p = piece_at_idx(63-(8*i+j),k);
				if(p >= 0){
					cout << piece_chars[k][p];
					break;
				}
			}
			if(p < 0) cout << "-";
		}
		cout << endl;
	}

	cout << '\n';

	if(basic) return;

	if(to_move == 0){
		a = 'w';
	} else {
		a = 'b';
	}
	cout << "to move: " << a << endl;
	cout << "en passant target: ";
	if(ep_target_square == 0){
		cout << "-" << endl;
	} else {
		cout << idx_to_coord(bit_to_idx(ep_target_square)) << endl;
	}
	cout << "zobrist: " << zobrist_key;
	cout << endl;
	return;
}

void chess_pos::dump_pos(ofstream& ofs) //for debugging
{
	int i,j,k;
	ofs << "POSDUMP:" << endl;

	for(i=0;i<=1;i++){
		ofs << "pl" << endl;
		for(j=0;j<__popcnt64(occ[i]);j++){
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
		ofs << "targ" << target_squares[i];
	}
	ofs << " capt" << captures;
	ofs << " lmce" << last_move_check_evasion;
	ofs << " lmc" << last_move_capture;
	ofs << " chsq" << changed_squares;
	ofs << " lmtf" << last_move_to_and_from;
	for(i=0;i<get_num_moves();i++){
		ofs << "	" << move_itos(pos_move_list.get_move(i));
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

int chess_pos::is_material_draw()
{
	int i,j,k,n;
	int eval = 0;
	
	// KvK, KBvK, KNvK, KdarkBvKlightB

	n = __popcnt64(occ[WHITE]+occ[BLACK]);

	if(n <= 2){
		return 1;
	} else if(n <= 3){
		if(__popcnt64(pieces[WHITE][KNIGHT]) == 1) return 1;
		if(__popcnt64(pieces[BLACK][KNIGHT]) == 1) return 1;
		if(__popcnt64(pieces[WHITE][BISHOP]) == 1) return 1;
		if(__popcnt64(pieces[BLACK][BISHOP]) == 1) return 1;
		return 0;
	} else if(n <= 4){
		if(__popcnt64(pieces[WHITE][BISHOP]) == 1) n--;
		if(__popcnt64(pieces[BLACK][BISHOP]) == 1) n--;
		if(n == 2){
			if(__popcnt64((pieces[BLACK][BISHOP] + pieces[WHITE][BISHOP]) & LIGHT_SQUARES) == 1){
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
			return CHECKMATE;
		} else {
			return -CHECKMATE;
		}
	} else {
		return STALEMATE;
	}
}

int chess_pos::get_num_moves()
{
	return pos_move_list.get_num_moves();
}
 

int chess_pos::eval()
{
	int eval = 0;
	int material_sum, material_diff;
	int p,n,b,r,q,P,N,B,R,Q;
	int i,j,k,white,black,wking,bking;
	int woffense, boffense;
	int king_safety;
	static const int targ_mult[KING] = {1,2,2,2,1};
	U64 temp;

	if(get_num_moves()<=0){
		return mate_eval();
	}

	p = int(__popcnt64(pieces[BLACK][PAWN]));
	n = int(__popcnt64(pieces[BLACK][KNIGHT]));
	b = int(__popcnt64(pieces[BLACK][BISHOP]));
	r = int(__popcnt64(pieces[BLACK][ROOK]));
	q = int(__popcnt64(pieces[BLACK][QUEEN]));
	P = int(__popcnt64(pieces[WHITE][PAWN]));
	N = int(__popcnt64(pieces[WHITE][KNIGHT]));
	B = int(__popcnt64(pieces[WHITE][BISHOP]));
	R = int(__popcnt64(pieces[WHITE][ROOK]));
	Q = int(__popcnt64(pieces[WHITE][QUEEN]));

	white =  int(__popcnt64(occ[WHITE]));
	black =  int(__popcnt64(occ[BLACK]));
	wking = bit_to_idx(pieces[WHITE][KING]);
	bking = bit_to_idx(pieces[BLACK][KING]);

	material_sum = P_MAT*(p+P)+N_MAT*(n+N)+B_MAT*(b+B)+R_MAT*(r+R)+Q_MAT*(q+Q);
	material_diff = P_MAT*(-p+P)+N_MAT*(-n+N)+B_MAT*(-b+B)+R_MAT*(-r+R)+Q_MAT*(-q+Q);
	eval += material_diff;

	if(abs(material_diff) >= 5*P_MAT && material_sum <= 2*Q_MAT){

		eval -= 2*int(__popcnt64(flood_fill_king(pieces[BLACK][KING],ctrl[WHITE]|occ[BLACK]|occ[WHITE],&MLUT,6)));
		eval += 2*int(__popcnt64(flood_fill_king(pieces[WHITE][KING],ctrl[BLACK]|occ[WHITE]|occ[BLACK],&MLUT,6)));
		eval += -material_diff/(P_MAT)*board_dist(wking,bking);

	} else {

		eval += (int(__popcnt64(ctrl[WHITE] & CENTER)) - int(__popcnt64(ctrl[BLACK] & CENTER)));

		if(material_sum > 5*R_MAT){
			eval += 2*int(__popcnt64(flood_fill_king(pieces[BLACK][KING],occ[BLACK]|occ[WHITE],&MLUT,4) & ctrl[WHITE]));
			eval -= 2*int(__popcnt64(flood_fill_king(pieces[WHITE][KING],occ[BLACK]|occ[WHITE],&MLUT,4) & ctrl[BLACK]));
		} else {
			eval += 2*int(__popcnt64(flood_fill_king(pieces[WHITE][KING],occ[BLACK]|ctrl[BLACK],&MLUT,3) & occ[WHITE]));
			eval -= 2*int(__popcnt64(flood_fill_king(pieces[BLACK][KING],occ[WHITE]|ctrl[WHITE],&MLUT,3) & occ[BLACK]));
		}

		woffense = int(__popcnt64(ctrl[WHITE]&occ[BLACK])) - int(__popcnt64(ctrl[BLACK]&occ[BLACK]));
		boffense = int(__popcnt64(ctrl[BLACK]&occ[WHITE])) - int(__popcnt64(ctrl[WHITE]&occ[WHITE]));

		eval += (woffense-boffense);

		for(i=1;i<white;i++){
			j = pl[WHITE][i].piece_type;
			if(j == PAWN) continue;
			eval += int(__popcnt64(pl[WHITE][i].targets))*targ_mult[j];
		}
		for(i=1;i<black;i++){
			j = pl[BLACK][i].piece_type;
			if(j == PAWN) continue;
			eval -= int(__popcnt64(pl[BLACK][i].targets))*targ_mult[j];
		}

	}

	eval += ((3*P_MAT)/2)*(int(__popcnt64(pieces[WHITE][PAWN] & (RANK_8 + RANK_7 + RANK_6))) - int(__popcnt64(pieces[BLACK][PAWN] & (RANK_1 + RANK_2 + RANK_3))));

	return EVAL_GRAIN*(eval/EVAL_GRAIN);
}