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
	resize(0);
}

U64 ttable::resize(U64 n_bytes)
{
	this->clear();
	U64 n_elements = n_bytes/(sizeof(tt_entry));
	n_elements = max((U64)256,n_elements);
	key_mask = n_elements;
	tt.reserve(n_elements);
	tt_entry invalid_entry = {0};
	tt.resize(n_elements, invalid_entry);
	return n_elements;
}

void ttable::place(const tt_entry &t)
{
	z_key key = t.full_key % key_mask;
	tt_entry curr = tt[key];
	if((curr.age >= t.age) && (curr.node_type > t.node_type)) return;
	tt[key] = t;
	return;
}

uint16_t ttable::find(z_key full_key, int* score, int* alpha, int* beta, int depth, short age) const
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
	return count/(tt.size()/1E6);
}

string ttable::extract_pv(chess_pos rpos, uint16_t first_move) const 
{
	string pv = move_itos(first_move);
	std::vector<z_key> past_pos;
	std::vector<z_key>::iterator it;
	z_key key, hkey;
	int idx;

	past_pos.push_back(rpos.zobrist_key);
	rpos.generate_moves();
	rpos.add_move(first_move);

	while(true){
		if((it = std::find(past_pos.begin(), past_pos.end(), rpos.zobrist_key)) != past_pos.end()) {
		    //repeated position in PV
		    idx = std::distance(it, past_pos.end());
		    pv += "\nLOOP IN PV LENGTH-" + to_string(idx);
		    break;
		}
		past_pos.push_back(rpos.zobrist_key);
		key = rpos.zobrist_key;
		hkey = key % key_mask;
		if(tt[hkey].full_key == key && tt[hkey].node_type == PVNODE){
			rpos.generate_moves();
			if(rpos.mList.swap_to_front(tt[hkey].best_move)){ //if move list contains this move
				rpos.add_move(tt[hkey].best_move);
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

void ttable::clear(){
	this->tt.clear();
}

U64 ttable::get_bytes() const{
	return tt.size()*sizeof(tt_entry); //bytes
}

U64 ttable::size() const{
	return tt.size(); 
}