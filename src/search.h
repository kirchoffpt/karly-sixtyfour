#ifndef SEARCH_H
#define SEARCH_H

#include "chess_pos.h"
#include "constants.h"
#include "ttable.h"
#include <thread>
#include <chrono>
#include <cstring>

struct search_options{
	unsigned short moves[MAX_MOVES_IN_POS];
	int num_moves;
	bool ponder;
	int time[2];							
	int inc[2], movestogo; 					// 0 for N/A
	int depth_limit, mate, movetime; 			// constraints, 0 for N/A
	unsigned long long nodes_limit;
	bool infinite;
};

class search_handler{
	chess_pos* rootpos;
	ttable* TT;
	public:
	//// UCI settings
	search_options uci_s;
	////
	bool is_searching;
	unsigned short best_move;
	unsigned short ponder_move; //move that we think the enemy will play after ours
	unsigned long long nodes_searched;
	int overall_top_score; //top score over all searches for position
	std::vector<z_key> past_positions;
	int search_depth;
	int search_id; //do not start search with id < 1
	std::vector<unsigned short> principal_variation;

	search_handler(chess_pos* rootpos);
	~search_handler();
	void go(); //launches search() threads
	void reset();
	void search(); 
	void stop();
	void max_timer(int ms, float incr);
	int quiesce(chess_pos* node, int depth, int color, int a, int b);
	int pvs(chess_pos* node, int depth, int color, int a, int b);
	int num_repetitions(const z_key position);
	bool allows_threefold(const chess_pos* c1);
	bool is_threefold(const chess_pos* c1);
};

#endif