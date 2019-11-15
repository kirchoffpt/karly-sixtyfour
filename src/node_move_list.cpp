#include "node_move_list.h"

#include <stdlib.h> //rand
#include <iostream>
#include <ctime>
#include "chess_funcs.h"

#ifndef U64
#define U64 unsigned long long int
#endif

#define PROMO_MOVE_BUNDLE 		0x0007000600050004
#define PROMO_MOVE_CONVOLUTE 	0x0001000100010001

//16 bit move, SRC-DST-SPECIAL-PROMOPIECE


using namespace std;

//FILO list

node_move_list::node_move_list()
{
	iterator = -1;
	srand(time(NULL));
}

void node_move_list::push_move(unsigned short move)
{
	moves[++iterator] = move;
	return;
}

void node_move_list::push_promo_move(unsigned short move)
{
	int i;
	*(U64*)(moves + iterator + 1) = PROMO_MOVE_BUNDLE + U64(move)*PROMO_MOVE_CONVOLUTE;
	iterator += 4;
	return;
}				

unsigned short node_move_list::pop_move()
{
	if(iterator >= 0){
		return moves[iterator--];
	} else {
		return 0;
	}
}

unsigned short node_move_list::pop_targeted_move(U64 targets)
{
	int i;
	unsigned short move_temp;

	if(iterator >= 0){
		for(i=get_num_moves()-1;i>=0;i--){
			if ((U64(1) << ((moves[i] & DST_MASK) >> DST_SHIFT)) & targets){
				move_temp = moves[iterator];
				moves[iterator] = moves[i];
				moves[i] = move_temp;
				return moves[iterator--];
			}
		}
		return 1;
	} else {
		return 0;
	}
}

unsigned short node_move_list::get_move(int move_list_idx)
{
	if((move_list_idx >= 0) && (move_list_idx <= iterator)){
		return moves[move_list_idx];
	} else {
		return 0;
	}
}

unsigned short node_move_list::get_random_move()
{
	if(iterator < 0) return 0;
	return moves[rand() % (iterator+1)];
}


int node_move_list::get_num_moves()
{
	return iterator+1;
}

void node_move_list::reset_num_moves()
{
	iterator = -1;
	return;
}

void node_move_list::sort_moves_by_scores(int* score_list)
{
	int i,j,max_idx;
	float score, max_score = SCORE_LO;
	unsigned short move_temp;

	for(j=get_num_moves()-1;j>0;j--){
		max_score = F_LO;
		for(i=j;i>=0;i--){
			score = *(score_list + i);
			if(score > max_score){
				max_score = score;
				max_idx = i;
			}
		}
		move_temp = moves[j];
		moves[j] = moves[max_idx];
		moves[max_idx] = move_temp;

		*(score_list+max_idx) =*(score_list+j);
		*(score_list+j) = max_score;
	}
	return;
}

unsigned short node_move_list::get_move_from_string(char* move_string)
{
	unsigned int idx, idx2;
	unsigned short move;
	char promos[4] = {'n','b','r','q'};
	int i,j,promotion = 3;

	idx = (7-(int(*move_string)-97))+8*(atoi(move_string + 1)-1);
	idx2 = (7-(int(*(move_string+2))-97))+8*(atoi(move_string + 3)-1);

	move = (idx << SRC_SHIFT) + (idx2 << DST_SHIFT);

	for(i=0;i<get_num_moves();i++){
		if((moves[i] & (SRC_MASK+DST_MASK)) == (move & (SRC_MASK+DST_MASK))){
			if((moves[i] & SPECIAL_MASK) == PROMO){
				for(j=0;j<4;j++){
					if(*(move_string+4) == promos[j]) promotion = j;
				}
				move += unsigned short(PROMO + promotion);
				return move;
			} else {
				return moves[i];
			}
		}
	}
	return 0;
}

void node_move_list::randomize_move_order(){
	int i,j,rand_idx;
	unsigned short move_temp;

	for(j=get_num_moves()-1;j>0;j--){
		rand_idx = rand() % (iterator+1);
		move_temp = moves[j];
		moves[j] = moves[rand_idx];
		moves[rand_idx] = move_temp;
	}
	return;
}

void node_move_list::swap_moves(int idx1, int idx2){
	unsigned short move_temp = moves[idx1];
	moves[idx1] = moves[idx2];
	moves[idx2] = move_temp;
	return; 
}