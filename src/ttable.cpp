#include "ttable.h"
#include <cstdio>
#include <iostream>

#ifndef U64
#define U64 unsigned long long int
#endif


using namespace std;


ttable::ttable()
{
	tt.reserve(0);
	tt.clear();
	next_placement = tt.begin();
}

U64 ttable::resize(U64 n_bytes)
{
	U64 n_elements = n_bytes/(sizeof(z_key)+sizeof(tt_entry)); 
	tt.reserve(n_elements);
	tt.clear();
	next_placement = tt.begin();
	max_elements = n_elements;
	return n_elements;
}

void ttable::place(z_key z, tt_entry t)
{
	if(tt.size() >= max_elements){
		//cout << tt.size() << " " << max_elements << endl;
		tt.erase(tt.begin(),tt.end());
	} else {
		tt.insert({z,t});
	}
}