#ifndef TTABLE_H
#define TTABLE_H

#ifndef U64
#define U64 unsigned long long int
#endif

#include <unordered_map>
#include "constants.h"


using namespace std;

struct tt_entry{
	unsigned short depth;
	int score;
	char node_type; //exact, upperbound, or lowerbound
	int age;
};

class ttable {
	public:
	std::unordered_map<z_key , tt_entry> tt;
	std::unordered_map<z_key , tt_entry>::iterator it; //for general use
	std::unordered_map<z_key , tt_entry>::iterator next_placement; //for targeting next open spot
	int max_elements;
	ttable();
	U64 resize(U64 n_bytes); //returns number of available table entries
	void place(z_key z, tt_entry t);
};


#endif