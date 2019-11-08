#ifndef SEARCH_H
#define SEARCH_H

#include "chess_pos.h"
#include "constants.h"
#include <thread>

using namespace std;

class search_handler{
  chess_pos* rootpos;
  //// UCI settings
  unsigned short moves[MAX_MOVES_IN_POS];
  int num_moves;
  bool ponder;
  int wtime, btime;             // can be negative
  int winc, binc, movestogo;          // 0 for N/A
  int depth_limit, mate, movetime;      // constraints, 0 for N/A
  unsigned long long nodes_limit;
  bool infinite;
  ////
  unsigned short best_move;
  unsigned short ponder_move; //move that we think the enemy will play after ours
  unsigned long long nodes_searched;
  int depth_searched;
  public:
  search_handler(chess_pos* rootpos);
  void set_time(int wt, int wi, int bt, int bi, int movestogo);
  void reset();
  void search(); 
  void stop();
  int quiesce(chess_pos* node, int min_or_max, int a, int b, int depth, int last_eval, int last_delta);
  int minimax(chess_pos* node, int depth, int min_or_max, int a, int b);
  int pvs(chess_pos* node, int depth, int min_or_max, int a, int b);
};

#endif