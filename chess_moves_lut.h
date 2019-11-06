#ifndef CHESS_MOVES_LUT_H
#define CHESS_MOVES_LUT_H

#include <map>
#include "constants.h"

#ifndef ll
#define ll unsigned long long int
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

using namespace std;

ll reverse_U64(ll x);

//lookup table for pseudo legal moves on a clear board but including pawn attacks. does not include castling

class chess_mask_LUT {
	ll move_mask[7][64];
	ll pawn_attack_mask[2][64];
	ll V_mask[2][64];
	ll L_mask[2][64];
	ll diag_ray[4][64];
	ll straight_ray[4][64];
	ll pawn_area_of_influence[2][64];
	ll rook_area_of_influence[64];
	ll en_passant_attackers[64];
	int piece_square_pawn[64];
	int piece_square_king[64];
	std::map<ll,ll> sliding_rays;

public:

	chess_mask_LUT();
	ll get_move_mask(int piece,int index); //piece from 0-6 (white_p,n,b,r,q,k,black_p), idx from 0-63
	ll get_pawn_attack_mask(int side, int index); //0 white, 1 black
	ll getVmask(bool reversed, int index); //V shaped or ^ shaped
	ll getLmask(bool reversed, int index); // _| shaped or |'' shaped
	ll get_sliding_ray(ll AB); //input is bitboard with occupations at A and B, returns 0 if angle is not multiple of 45 degrees
	ll get_diag_ray(int direction, int index); //SE, SW, NW, NE
	ll get_straight_ray(int direction, int index); //right, bottom, left, top
	ll get_pawn_area_of_influence(int side, int index); //3x5 rectangle around king
	ll get_rook_area_of_influence(int index);
	ll get_en_passant_attackers(int index);
	int get_piece_square_pawn(int index);
	int get_piece_square_king(int index);
};


#endif