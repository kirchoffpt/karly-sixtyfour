#ifndef CHESS_POS_H
#define CHESS_POS_H

#include <cmath>
#include <time.h>
#include <Windows.h>
#include <cstdio>
#include <iomanip>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <fstream>
#include <string>
#include <intrin.h> //popcnt and bitscan
#include "immintrin.h"
#include "assert.h"
#include <ctime>
#include "chess_moves_lut.h"
#include "node_move_list.h"
#include "constants.h"
#include "chess_funcs.h"

#define U64 unsigned long long int

using namespace std;

struct piece_list_struct 
{ 
	int piece_type; 	
	int pinned;    //1 or 0
	U64 loc; 		//one set bit, location of piece
	U64 ctrl_sq;	//all squares attacked that enemy king may not move into
	U64 targets;  	//all possible move locations plus blocker locations
};


class chess_pos {
	public:
	unsigned short id;
	piece_list_struct pl[2][MAX_PIECES_PER_SIDE];
	U64 pin_rays[2][8]; //8 for each king, 0-7, E,SE,S,SW,W,NW,N,NE
	U64 pieces[2][6]; 
	U64 occ[2]; //0 for w, 1 for b
	int piece_at[64]; //undefined if no piece on idx! use piece_at_idx() instead
	U64 ep_target_square;
	U64 castlable_rooks;
	z_key zobrist_key;
	int to_move; // '0' or '1'
	int in_check; //0 not in check. 1 in check. 2 double check.
	//-----------------------------
	//------------------------------
	U64 captures; //enemy pieces that may be captured
	int last_move_check_evasion;
	int last_move_capture;
	int evaluation;
	U64 changed_squares;
	U64 last_move_to_and_from;
	U64 ctrl[2];
	U64 target_squares[2];
	node_move_list pos_move_list;
	chess_pos* next; //next node
	chess_pos* prev; //prev node, for getting eval deltas

	chess_pos(string);
	chess_pos();
	void load_new_fen(string);
	int is_quiet();
	int eval();
	int mate_eval(); //moves must have been generated first
	void add_move(unsigned short move); //make move at top of this positions move list
	void add_move_to_next_node(unsigned short move); //copy this position into another and add move there
	void print_pos(bool);
	void dump_pos(ofstream& ofile); //for debugging
	void init_piece_list();
	void init_zobrist();
	void generate_moves(); 
	void generate_moves_deprecated(); //deprecated, but should generate moves correctly
	void init_targets(int side);
	void copy_pos(chess_pos* source_pos); //purely the position and castling/enpassant rights
	int is_material_draw(); // KvK, KBvK, KNvK, KdarkBvKlightB
	int piece_at_idx(int idx, int side); //returns -1 if none
	unsigned short pop_and_add(); //applies move, decrements number of moves, and COPIES POS TO NEXT NODE. returns move or 0 when out of moves or no next node
	unsigned short pop_and_add_capture(); //returns 0 if no moves, returns 1 if no captures
	U64 prune_blocked_moves(int piece_type, int center, U64* move_mask, U64 blockers); //returns bitboard of blocking pieces
	int get_num_moves();
	void store_init_targets(U64 piece_loc, U64 targets, int pinned); //into piece list
	U64 create_pawn_pushes(U64 pawn_loc, int side);
	void sort_piece_list(); //not meant to be fast. use only at very low depths. 
							//helps speed up search and mostly remove ambiguities between uci position input methods (fen + added moves VS just a fen )

};


#endif