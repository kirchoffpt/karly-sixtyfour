#ifndef POSTREE_H
#define POSTREE_H

//linked list of chess pos

#include <vector>
#include <memory>
#include "chess_pos.h"

class postree {
	private:
	std::vector<chess_pos*> nodes;
	
	public:
	postree(chess_pos rootpos, int max_depth); 
	~postree();
	chess_pos* root();
};


#endif