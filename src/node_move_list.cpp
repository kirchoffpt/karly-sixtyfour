#include "node_move_list.h"

#include <stdlib.h> //rand
#include <iostream>
#include <ctime>
#include <cstring>
#include "assert.h"
#include "chess_funcs.h"

#ifndef U64
#define U64 unsigned long long int
#endif

#define PROMO_MOVE_BUNDLE 		0x0007000600050004
#define PROMO_MOVE_CONVOLUTE 	0x0001000100010001

//16 bit move, SRC-DST-SPECIAL-PROMOPIECE
//6-6-2-2 total 16


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
	*(U64*)(moves + iterator + 1) = PROMO_MOVE_BUNDLE + (U64)(move)*PROMO_MOVE_CONVOLUTE;
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
			if (((U64)(1) << ((moves[i] & DST_MASK) >> DST_SHIFT)) & targets){
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


unsigned int node_move_list::get_num_moves()
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
	int score, max_score = SCORE_LO;
	unsigned short move_temp;

	for(j=get_num_moves()-1;j>0;j--){
		max_score = SCORE_LO;
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

unsigned short node_move_list::get_move_from_string(string move_string)
{
	unsigned int idx, idx2;
	unsigned short move;
	char promos[4] = {'n','b','r','q'};
	uint32_t i,j,promotion = 3;

	move = encode_move_srcdst(move_string);

	for(i=0;i<get_num_moves();i++){
		if((moves[i] & (SRC_MASK+DST_MASK)) == (move & (SRC_MASK+DST_MASK))){
			if((moves[i] & SPECIAL_MASK) == PROMO){
				for(j=0;j<4;j++){
					if(move_string[4] == promos[j]) promotion = j;
				}
				move += (unsigned short)(PROMO + promotion);
				return move;
			} else {
				return moves[i];
			}
		}
	}
	return 0;
}

void node_move_list::randomize_move_order(){
	int j,rand_idx;
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
	std::swap(moves[idx1],moves[idx2]);
	return; 
}

bool node_move_list::swap_to_front(unsigned short move){
	int i = get_num_moves()-1;
	if(moves[i--] == move) return true;
	for(;i>=0;i--){
		if(move == moves[i]){
			swap_moves(i,iterator);
			return true;
		}
	}
	return false;
}

bool node_move_list::move_to_front(unsigned short move){
	int i = get_num_moves()-1;
	int j, chksum = 0; //for debug since memcpy on overlapped objects is undefined
	if(DEBUG){
		for(j=0;j<=iterator;j++) chksum += moves[j];
	}
	if(moves[i--] == move) return true;
	for(;i>=0;i--){
		if(move == moves[i]){
			memcpy(moves+i,moves+i+1,sizeof(move)*(iterator-i));
			moves[iterator] = move;
			if(DEBUG){
				for(j=0;j<=iterator;j++) chksum -= moves[j];
				try{
					if(chksum != 0) throw chksum;
				}
				catch(int e){
					cout << "move_to_front failed" << endl;
					assert(chksum == 0);
				}
			}
			return true;
		}
	}
	return false;
}

void node_move_list::print_moves(){
	int i = get_num_moves()-1;
	for(;i>=0;i--){
		cout << move_itos(moves[i]) + " | ";
	}
	cout << get_num_moves() << endl;
	return;
}

int node_move_list::contains(unsigned short move){
	int i = get_num_moves()-1;
	for(;i>=0;i--){
		if(moves[i] == move) break;
	}
	return i;
}