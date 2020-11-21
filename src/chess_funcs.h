#pragma once
#include <string>
#include "constants.h"

#define U64 uint64_t


void print_bitboard(U64 board);
void print_bitboard(U64 board, std::ofstream& ofs);
bool is_sliding_piece(int piece_type);
std::string piece_itos(int piece_type);
std::string node_itos(int node_type);
uint32_t bit_to_idx(U64 bitboard);
void print_move(uint16_t move);
std::string move_itos(uint16_t move);
std::string idx_to_coord(int idx);
int get_ray_dir(int a_idx, int b_idx); //returns 0-7 corresponding to directions E to NE clockwise
U64 flood_fill_king(U64 king_loc, U64 enemy_control, int depth); //gives all the squares a king could travel to given so many moves in a row
int board_dist(int idx1, int idx2);
uint16_t encode_move_srcdst(std::string move_str); //input example "e2e4", discards chars afer 4th
uint16_t encode_move_srcdst(int src_idx, int dst_idx);

