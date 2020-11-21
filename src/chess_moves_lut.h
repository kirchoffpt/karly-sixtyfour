#ifndef CHESS_MOVES_LUT_H
#define CHESS_MOVES_LUT_H

#include <map>
#include "constants.h"

#ifndef U64
#define U64 uint64_t
#endif

//  A B C D E F G H
//8 63
//7
//6
//5
//4
//3
//2
//1               0

U64 reverse_U64(U64 x);

//lookup table for pseudo legal moves on a clear board but including pawn attacks. does not include castling

class chess_mask_LUT {

private:

	U64 move_mask[7][64];
	U64 pawn_attack_mask[2][64];
	U64 V_mask[2][64];
	U64 L_mask[2][64];
	U64 diag_ray[4][64];
	U64 straight_ray[4][64];
	U64 pawn_area_of_influence[2][64];
	U64 rook_area_of_influence[64];
	U64 en_passant_attackers[64];
	int piece_square_pawn[64];
	int piece_square_king[64];
	std::map<U64,U64> sliding_rays;
	z_key zobrist_piece[2][6][64];
	z_key zobrist_black_to_move;

public:
	chess_mask_LUT();
	U64 get_move_mask(int piece,int index); //piece from 0-6 (white_p,n,b,r,q,k,black_p), idx from 0-63
	U64 get_pawn_attack_mask(int side, int index); //0 white, 1 black
	U64 getVmask(bool reversed, int index); //V shaped or ^ shaped
	U64 getLmask(bool reversed, int index); // _| shaped or |'' shaped
	U64 get_sliding_ray(U64 AB); //input is bitboard with occupations at A and B, returns 0 if angle is not multiple of 45 degrees
	U64 get_diag_ray(int direction, int index); //SE, SW, NW, NE
	U64 get_straight_ray(int direction, int index); //right, bottom, left, top
	U64 get_pawn_area_of_influence(int side, int index); //3x5 rectangle around king
	U64 get_rook_area_of_influence(int index);
	U64 get_en_passant_attackers(int index);
	U64 get_zobrist_piece(int side, int piece_type, int idx);
	U64 get_zobrist_btm();
};


#endif