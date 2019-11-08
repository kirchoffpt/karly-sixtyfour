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
#include <ctime>
#include "constants.h"

#define U64 unsigned long long int

using namespace std;

void print_bitboard(U64 board);
void print_bitboard(U64 board, ofstream& ofs);
bool is_sliding_piece(int piece_type);
string piece_itos(int piece_type);
int bit_to_idx(U64 bitboard);
void print_move(unsigned short move);
string move_itos(unsigned short move);
string idx_to_coord(int idx);
int get_ray_dir(int a_idx, int b_idx); //returns 0-7 corresponding to directions E to NE clockwise

