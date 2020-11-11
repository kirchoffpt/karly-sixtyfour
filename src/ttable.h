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
	uint16_t best_move;
	char node_type; //exact, upperbound, or lowerbound
};

class ttable {
	z_key key_mask = ~z_key(0);
	std::vector<tt_entry> tt;
	void clear();
	
	public:
	ttable();
	U64 resize(U64 n_bytes); //clears table, returns number of available table entries
	U64 get_bytes() const;
	void reset(){ resize(get_bytes()); };
	U64 size() const;
	uint16_t find(z_key full_key, int* score, int* alpha, int* beta, int depth, short age) const; //returns best move for position
	void place(const tt_entry &t); 
	int hashfull() const; //returns valid elements per million, slow
	std::string extract_pv(chess_pos rpos, uint16_t first_move) const;
	void dump_table(std::ostream &os);
};


#endif