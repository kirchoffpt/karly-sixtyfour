#ifndef NODE_MOVE_LIST_H
#define NODE_MOVE_LIST_H

#ifndef U64
#define U64 uint64_t
#endif

#include "constants.h"
#include <string>

//FILO list

class node_move_list {
	uint16_t moves[MAX_MOVES_IN_POS];
	int iterator;
public:
	node_move_list();
	void push_move(uint16_t move); 
	void push_promo_move(uint16_t move); //only src and destination are needed
	uint16_t pop_move(); //returns 0 if no move
	uint16_t pop_targeted_move(U64 targets); //pops first move with a dst in targets. returns 0 if no moves in list, returns 1 if no targeted move available
	uint16_t get_move(int move_list_idx);
	uint16_t get_random_move();
	unsigned int get_num_moves();
	void reset_num_moves();
	void sort_moves_by_scores(int* score_list);
	uint16_t get_move_from_string(std::string move_string);
	void randomize_move_order();
	void swap_moves(int idx1, int idx2);
	bool swap_to_front(uint16_t move); //simple swapping of elements
	bool move_to_front(uint16_t move); //preserves order of other elements
	void print_moves();
	int contains(uint16_t move); //returns idx, -1 if not found
	const uint16_t* data(){return moves;} //if explicit reading of moves is needed
};


#endif