#ifndef SEARCH_H
#define SEARCH_H

#include "chess_pos.h"
#include "constants.h"
#include "ttable.h"
#include <thread>
#include <chrono>

using namespace std;

struct search_options{
	unsigned short moves[MAX_MOVES_IN_POS];
	int num_moves;
	bool ponder;
	int time[2];							// can be negative
	int inc[2], movestogo; 					// 0 for N/A
	int depth_limit, mate, movetime; 			// constraints, 0 for N/A
	unsigned long long nodes_limit;
	bool infinite;
};

class search_handler{
	chess_pos* rootpos;
	public:
	//// UCI settings
	search_options uci_s;
	////
	bool is_searching;
	unsigned short best_move;
	unsigned short ponder_move; //move that we think the enemy will play after ours
	unsigned long long nodes_searched;
	int depth_searched;
	std::vector<z_key> past_positions;
	ttable* TT;
	int t_depth;
	int search_id;
	std::vector<unsigned short> pv_moves;

	search_handler(chess_pos* rootpos);
	void go(); //launches search() threads
	void reset();
	void search(); 
	void stop(int id);
	void max_timer(int ms, int id);
	int quiesce(chess_pos* node, int min_or_max, int a, int b, int depth, int last_eval, int last_delta);
	int minimax(chess_pos* node, int depth, int min_or_max, int a, int b);
	int pvs(chess_pos* node, int depth, int min_or_max, int a, int b);
	bool is_threefold_repetition(z_key position);
};

#endif