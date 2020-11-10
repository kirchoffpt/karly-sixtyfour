#ifndef TTABLE_H
#define TTABLE_H

#ifndef U64
#define U64 uint64_t
#endif

#include <vector>
#include <string>
#include "chess_pos.h"
#include "constants.h"

struct tt_entry{
	z_key full_key;
	int score;
	short age;
	short depth;
	unsigned short best_move;
	char node_type; //exact, upperbound, or lowerbound
};

class ttable {
	z_key key_mask = ~z_key(0);
	
	public:
	std::vector<tt_entry> tt;
	U64 max_elements;
	ttable();
	U64 resize(U64 n_bytes); //returns number of available table entries
	unsigned short find(z_key full_key, int* score, int* alpha, int* beta, int depth, unsigned short age); //returns best move for position
	void place(z_key z, tt_entry &t); 
	int hashfull() const; //returns valid elements, slow
	std::string extract_pv(const chess_pos* rpos, unsigned short first_move) const;
	void dump_table(std::ostream &os);
};


#endif