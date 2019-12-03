#include "ttable.h"
#include <cstdio>
#include <iostream>
#include <math.h>
#include <algorithm>

#ifndef U64
#define U64 unsigned long long int
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
	key_mask = n_elements;
	tt.reserve(n_elements);
	tt_entry invalid_entry = {0};
	tt.resize(n_elements, invalid_entry);
	max_elements = n_elements;
	return n_elements;
}

void ttable::place(z_key z, tt_entry t)
{
	t.full_key = z;
	tt[z % key_mask] = t;
}

unsigned short ttable::find(z_key full_key, int* score, int* alpha, int* beta, int depth)
{
	z_key key = full_key % key_mask;
	if(tt[key].full_key == full_key){
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
	return 0;
}

int ttable::hashfull()
{
	U64 i, count = 0;
	for(i=0;i<tt.size();i++){
		if(tt[i].depth != 0) count++;
	}
	return count/(tt.size()/1000000);
}