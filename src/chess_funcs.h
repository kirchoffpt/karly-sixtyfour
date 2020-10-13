#include <cmath>
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
#include <ctime>
#include "constants.h"
#include "chess_moves_lut.h"

#define U64 unsigned long long int

void print_bitboard(U64 board);
void print_bitboard(U64 board, std::ofstream& ofs);
bool is_sliding_piece(int piece_type);
std::string piece_itos(int piece_type);
std::string node_itos(int node_type);
int bit_to_idx(U64 bitboard);
void print_move(unsigned short move);
std::string move_itos(unsigned short move);
std::string idx_to_coord(int idx);
int get_ray_dir(int a_idx, int b_idx); //returns 0-7 corresponding to directions E to NE clockwise
U64 flood_fill_king(U64 king_loc, U64 enemy_control, chess_mask_LUT* mlut, int depth); //gives all the squares a king could travel to given so many moves in a row
int board_dist(int idx1, int idx2);

