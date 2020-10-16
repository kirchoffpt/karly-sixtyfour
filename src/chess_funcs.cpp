
#include "chess_funcs.h"
#include "bitops.h"

using namespace std;

void print_bitboard(U64 board)
{
	int i,j;

	cout << endl;

	for(i=0,j=0;i<64;i++){
		if(j==0) cout << endl;
		if ((board & (0x8000000000000000))==0x8000000000000000){
			cout << 'x';
		} else {
			cout << '-';
		}
		board <<= 1;
		j = (j+1)%8;
	}

	cout << endl;

	return;
}

void print_bitboard(U64 board, ofstream& ofs)
{
	int i,j;

	ofs << endl;

	for(i=0,j=0;i<64;i++){
		if(j==0) ofs << endl;
		if ((board & (0x8000000000000000))==0x8000000000000000){
			ofs << 'x';
		} else {
			ofs << '-';
		}
		board <<= 1;
		j = (j+1)%8;
	}

	ofs << endl;

	return;
}

bool is_sliding_piece(int piece_type)
{
	if ((piece_type == BISHOP)||(piece_type == ROOK)||(piece_type == QUEEN)) return true;
	return false;
}

string piece_itos(int piece_type)
{
	switch(piece_type){
		case PAWN 	:	return "PAWN";
			break;
		case KNIGHT :	return "KNIGHT";
			break;
		case BISHOP	:	return "BISHOP";
			break;
		case ROOK	:	return "ROOK";
			break;
		case QUEEN	:	return "QUEEN";
			break;
		case KING 	:	return "KING";
	}
	return "UNDEFINED_PIECE_TYPE";
}

string node_itos(int node_type)
{
	switch(node_type){
		case PVNODE		:	return "P!";
			break;
		case ALLNODE 	:	return "A";
			break;
		case CUTNODE	:	return "C";
	}
	return "UNDEFINED_NODE_TYPE";
}

uint32_t bit_to_idx(U64 bitboard)
{
	unsigned long idx;

	bitops::bscanf64( &idx, bitboard);

	return int(idx);
}

string idx_to_coord(int idx)
{
	int x = idx%8;
	int y = idx/8;
	string coord = "";

	y = y + 1;

	coord.append(sizeof(char),("abcdefgh"[7-x]));
	coord.append(to_string(y));

	return coord;
}

void print_move(unsigned short move)
{
	cout << move_itos(move);
}

string move_itos(unsigned short move)
{
	unsigned short x;
	if(move == 0) return "0000";
	string s = "";
	s += idx_to_coord((move & SRC_MASK) >> SRC_SHIFT);
	s += idx_to_coord((move & DST_MASK) >> DST_SHIFT);
	x = move & SPECIAL_MASK;
	if(x == PROMO){
		//" nbrq"[move & PROMO_MASK];
		s += "nbrq"[move & PROMO_MASK];
	}
	return s;
}

int get_ray_dir(int a, int b) //a,b indexes
{
	int x, y;
	
	x = a%8 - b%8;
	y = b/8 - a/8;

	if(b > a){ //W, NW, N, NE
		if(x < 0 ){
			if(y == 0){
				return W_DIR;
			} else {
				return NW_DIR;
			}
		} else { 
			if(x == 0){
				return N_DIR;
			} else{
				return NE_DIR;
			}
		}
	} else { // E, SE, S, SW 
		if(x > 0){
			if(y == 0){
				return E_DIR;
			} else {
				return SE_DIR;
			}
		} else {
			if(x == 0){
				return S_DIR;
			} else{
				return SW_DIR;
			}
		}
	}
	return -1;
}

U64 flood_fill_king(U64 king_loc, U64 enemy_control, chess_mask_LUT* mlut, int depth)
{
	U64 fill = king_loc;
	U64 temp, done_sq = 0;
	unsigned long idx;
	
	for(;depth>0;depth--){
		temp = fill &~ done_sq;
		done_sq |= temp;
		while(bitops::bscanf64(&idx, temp)){
			temp ^= (U64)1 << idx;
			fill |= mlut->get_move_mask(KING,idx);
		}
		fill &= ~enemy_control;
		if(fill == done_sq) break;
	}
	return fill;
}

int board_dist(int idx1, int idx2)
{
	int x,y;

	x = abs(idx1%8-idx2%8);
	y = abs(idx1/8-idx2/8);

	return x + y;
}

