#include "ttable.h"
#include <cstdio>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <cstdint>
#include "chess_funcs.h"

#ifndef U64
#define U64 uint64_t
#endif


using namespace std;


ttable::ttable()
{
	tt.reserve(0);
	tt.clear();
}

U64 ttable::resize(U64 n_bytes)
{
	U64 n_elements = n_bytes/(sizeof(tt_entry));
	n_elements = max((U64)5,n_elements);
	key_mask = n_elements;
	tt.reserve(n_elements);
	tt_entry invalid_entry = {0};
	tt.resize(n_elements, invalid_entry);
	max_elements = n_elements;
	return n_elements;
}

void ttable::place(z_key z, tt_entry &t)
{
	z_key key = z % key_mask;
	tt_entry curr = tt[key];
	if((curr.age >= t.age) && (curr.node_type > t.node_type)) return;
	t.full_key = z;
	tt[key] = t;
	return;
}

unsigned short ttable::find(z_key full_key, int* score, int* alpha, int* beta, int depth, unsigned short age)
{
	z_key key = full_key % key_mask;
	if(tt[key].full_key == full_key){
		if(age == tt[key].age){
			if(int(tt[key].depth) >= depth){
				switch(tt[key].node_type){
					case PVNODE:
						*score = tt[key].score;
						break;
					case CUTNODE:
						*alpha = max(*alpha, tt[key].score);
						break;
					case ALLNODE:
						*beta = min(*beta, tt[key].score);
				}
			}
			return tt[key].best_move;
		}
		if(USE_OLD_TTABLE_BEST_MOVES) return tt[key].best_move;
	}
	return 0;
}

int ttable::hashfull() const
{
	U64 i, count = 0;
	for(i=0;i<tt.size();i++){
		if(tt[i].age != 0) count++;
	}
	return count/(tt.size()/100);
}

string ttable::extract_pv(const chess_pos* rpos, unsigned short first_move) const 
{
	chess_pos pv_pos;
	string pv = move_itos(first_move);
	std::vector<z_key> past_pos;
	std::vector<z_key>::iterator it;
	z_key key, hkey;
	pv_pos = *const_cast<chess_pos*>(rpos);
	int idx;

	past_pos.push_back(pv_pos.zobrist_key);
	pv_pos.generate_moves();
	pv_pos.add_move(first_move);

	while(true){
		if((it = std::find(past_pos.begin(), past_pos.end(), pv_pos.zobrist_key)) != past_pos.end()) {
		    //repeated position in PV
		    idx = std::distance(it, past_pos.end());
		    pv += "\nLOOP IN PV LENGTH-" + to_string(idx);
		    break;
		}
		past_pos.push_back(pv_pos.zobrist_key);
		key = pv_pos.zobrist_key;
		hkey = key % key_mask;
		if(tt[hkey].full_key == key && tt[hkey].node_type == PVNODE){
			pv_pos.generate_moves();
			if(pv_pos.pos_move_list.swap_to_front(tt[hkey].best_move)){ //if move list contains this move
				pv_pos.add_move(tt[hkey].best_move);
				pv += " " + move_itos(tt[hkey].best_move);
			} else {
				goto exit_pv_loop;
			}
		} else {
			break;
		}
	}

	exit_pv_loop:

	return pv;
}

void ttable::dump_table(ostream &os){
	unsigned long long i;

	for(i=0;i<key_mask;i++){
		if(tt[i].age != 0){
			os << hex << i;
			os << dec << "	n: " << node_itos(tt[i].node_type);
			os << "	s: " << tt[i].score;
			os << "	b: " << move_itos(tt[i].best_move);
			os << "	d: " << tt[i].depth;
			os << "	a: " << tt[i].age;
			os << endl;
		}
	}
	return;
}