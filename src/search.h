#ifndef SEARCH_H
#define SEARCH_H

#include "chess_pos.h"
#include "constants.h"
#include "ttable.h"

struct search_options{
	uint16_t moves[MAX_MOVES_IN_POS];
	int num_moves;
	bool ponder;
	int time[2];							
	int inc[2], movestogo; 					// 0 for N/A
	int depth_limit, mate, movetime; 			// constraints, 0 for N/A
	unsigned long long nodes_limit;
	bool infinite;
};

class search_handler{
	private:
	chess_pos* rootpos;
	ttable* TT;
	unsigned int search_id = 0;
	
	public:
	//// UCI settings
	search_options uci_s;
	////
	bool is_searching;
	uint16_t best_move;
	uint16_t ponder_move; //move that we think the enemy will play after ours
	unsigned long long nodes_searched;
	int overall_top_score; //top score over all searches for position
	std::vector<z_key> past_positions;
	int search_depth;
	std::vector<uint16_t> principal_variation;

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
	const ttable* get_ttable(){return TT;};
};

#endif