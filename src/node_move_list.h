#ifndef NODE_MOVE_LIST_H
#define NODE_MOVE_LIST_H

#ifndef U64
#define U64 unsigned long long int
#endif

#include "constants.h"


using namespace std;

//FILO list

class node_move_list {
	unsigned short moves[MAX_MOVES_IN_POS];
	int iterator;
public:
	node_move_list();
	void push_move(unsigned short move); 
	void push_promo_move(unsigned short move); //only src and destination are needed
	unsigned short pop_move(); //returns 0 if no move
	unsigned short pop_targeted_move(U64 targets); //pops first move with a dst in targets. returns 0 if no moves in list, returns 1 if no targeted move available
	unsigned short get_move(int move_list_idx);
	unsigned short get_random_move();
	int get_num_moves();
	void reset_num_moves();
	void sort_moves_by_scores(int* score_list);
	unsigned short get_move_from_string(char* move_string);
	void randomize_move_order();
	void swap_moves(int idx1, int idx2);
};


#endif