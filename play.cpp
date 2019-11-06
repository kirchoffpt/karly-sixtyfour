#include "chess_pos.h"

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

#define MAX_MC_DEPTH 100
#define MAX_AB_DEPTH 24
#define MIN_DEPTH 5
#define MAX_Q_DEPTH 24

using namespace std;

int global_n = 0;

int quiesce(chess_pos* node, int min_or_max, int a, int b, int depth, int last_eval, int last_delta){
	global_n++;

	int eval, eval_temp, stand_pat, delta;
	short move;

	if(last_delta == SCORE_LO){
		stand_pat = node->eval();
		delta = stand_pat - last_eval;
		last_delta = delta;
	} else {
		node->generate_moves();
		stand_pat = node->eval();
		delta = stand_pat - last_eval;
	}


	// node->print_pos(true);
	// print_bitboard(node->captures);
	// cout << node->eval()/100.0 <<  "	depth: " << depth << endl;
	// cout << delta << "	" << last_delta << endl;
	// Sleep(500);

	if(node->get_num_moves() <= 0){
		return node->mate_eval();
	}

	eval = stand_pat;
	if(min_or_max){
			b = min(b, eval);
		} else {
			a = max(a, eval);
		}
	if(a >= b) return eval;

	// cout << "depth: " << depth << endl;
	// node->print_pos(false);
	// print_bitboard(node->captures);


	if(node->captures == 0 || depth == 0){
		// cout << "---------------: " << node->eval() << endl;
		// Sleep(250);
		return stand_pat;
	} else {
		if(min_or_max){
			if(delta < -(last_delta+Q_DELTA_WINDOW)){
				//cout << "STND PATl: " << stand_pat << endl;
				return stand_pat;		
			} 
		} else {
			if(delta > -(last_delta-Q_DELTA_WINDOW)){
				//cout << "STND PATg: " << stand_pat << endl;
				return stand_pat;
			} 
		}
	}


	
	if(min_or_max){
		while(node->pop_and_add_capture() != 1){
			eval_temp = quiesce(node->next, FALSE, a, b, depth-1, stand_pat, delta);
			eval = min(eval_temp,eval);
			b = min(b, eval);
			if(a >= b) break;
			if(node->get_num_moves() <= 0) break;
		}
	} else {
		while(node->pop_and_add_capture() != 1){
			eval_temp = quiesce(node->next, TRUE, a, b, depth-1, stand_pat, delta);
			eval = max(eval_temp, eval);
			a = max(a, eval);
			if(a >= b) break;
			if(node->get_num_moves() <= 0) break;
		}
	}
    return eval;
}

int minimax(chess_pos* node, int depth, int min_or_max, int a, int b){
	global_n++;
	node->generate_moves();
	if(depth == 0){
		if(node->captures > 0){
			/////
			//node->print_pos(true);
			return quiesce(node,min_or_max,a,b,MAX_Q_DEPTH,node->prev->eval(),SCORE_LO);
		}
		if(node->last_move_check_evasion || node->in_check){
			/////
			//return minimax(node,1,min_or_max,a,b);
		}		
		return node->eval();
	}

	int eval, eval_temp;
	if(node->pop_and_add()){
		eval = minimax(node->next, depth-1, !min_or_max, a, b);
		if(min_or_max){
			b = min(b, eval);
		} else {
			a = max(a, eval);
		}
		if(a >= b) return eval;
	} else {
		return node->mate_eval();
	}
	if(min_or_max){
		while(node->pop_and_add()){
			eval_temp = minimax(node->next, depth-1, FALSE, a, b);
			eval = min(eval_temp,eval);
			b = min(b, eval);
			if(a >= b) break;
		}
	} else {
		while(node->pop_and_add()){
			eval_temp = minimax(node->next, depth-1, TRUE, a, b);
			eval = max(eval_temp, eval);
			a = max(a, eval);
			if(a >= b) break;
		}
	}
	return eval;
}

int pvs(chess_pos* node, int depth, int min_or_max, int a, int b){
    node->generate_moves();
	if(depth == 0){
		global_n++;
		if(min_or_max){
			return -node->eval();
		} else {
			return node->eval();
		}
	}
	int eval, eval_temp;
	if(node->pop_and_add()){
		eval = -pvs(node->next, depth - 1, !min_or_max, -b,-a);
		a = max(a, eval);
        if(a >= b) return a;
	} else {
		global_n++;
		if(min_or_max){
			return -node->mate_eval();
		} else {
			return node->mate_eval();
		}
	}
    while(node->pop_and_add()){
    	eval = -pvs(node->next, depth - 1, !min_or_max, -a - 1, -a);
        if((a < eval) && (eval < b)) eval = -pvs(node->next, depth - 1,!min_or_max, -b, -eval);
        a = max(a, eval);
        if(a >= b) break;
    }
    return a;
}

int main(int argc, char *argv[]){
	int i,j,n_root_moves;
	int to_move_sign;
	int score, depth_score, top_score, alpha, beta;
	chess_pos* node_ptrs[MAX_AB_DEPTH+MAX_Q_DEPTH+1];
	unsigned short move, top_move;
	int move_scores [MAX_MOVES_IN_POS] = {0};
	int MC_scores [MAX_MOVES_IN_POS] = {0};
	int depth, max_depth = MAX_AB_DEPTH;
	int to_move;
	long long k = 0;
	float t, tt;
	char move_string[5];
	clock_t cl;     //initializing a clock type

	chess_pos rootpos(argv[1]);
	chess_pos MC_node;

	i = MAX_AB_DEPTH+MAX_Q_DEPTH;
	node_ptrs[i--] = new chess_pos;
	for(i;i>=0;i--){
		node_ptrs[i] = new chess_pos;
		node_ptrs[i]->next = node_ptrs[i+1];
		node_ptrs[i]->id = i; 
	}
	rootpos.next = node_ptrs[0];
	node_ptrs[0]->prev = &rootpos;
	for(i=1;i<=MAX_AB_DEPTH+MAX_Q_DEPTH;i++){
		node_ptrs[i]->prev = node_ptrs[i-1];
	}

	while(1){
	rootpos.generate_moves();
	n_root_moves = rootpos.get_num_moves();
	to_move_sign = ((!rootpos.to_move)*2-1);


    tt = 0;
    max_depth = MAX_AB_DEPTH;
	for(depth=min(max_depth,1);depth <= max_depth;depth++){
		top_score = SCORE_LO;
		alpha = SCORE_LO;
		beta =  SCORE_HI;
		cout << "----------d" << depth << endl;
		for(i=n_root_moves-1;i>=0;i--){
			move = rootpos.pos_move_list.get_move(i);
			//if((33<<SRC_SHIFT) + (40<<DST_SHIFT) != move) continue;
			rootpos.next->copy_pos(&rootpos);
			rootpos.next->add_move(move);
			global_n = 0;
			cl = clock(); 
			score = minimax(rootpos.next, depth-1,!rootpos.to_move, alpha, beta);
			if(rootpos.to_move){
				beta = min(beta, score);
			} else {
				alpha = max(alpha, score);
			}
			cl = clock() - cl;
			t = cl/(double)CLOCKS_PER_SEC;
			tt += t;
			move_scores[i] = score*to_move_sign;
			print_move(move);
			cout << "	score: ";
			if(score > 0){
				cout << "+";
			}
			cout << float(score)/100.0;
			cout << "	time: " <<  t;
			if(t > 0.005) cout << "	kNPS: " << (0.001*global_n)/t;
			cout << endl;
			if(score*to_move_sign > top_score){
				top_move = move;
				top_score = score*to_move_sign;
			}
			if(top_score >= CHECKMATE){
				goto exit_minimax_loop;
			}
		}
		cout << endl;
		rootpos.pos_move_list.sort_moves_by_scores(move_scores);
		if(tt > 0.5 && depth >= MIN_DEPTH) break;
	}
	exit_minimax_loop:

	cout << endl;
	cout << "top_move: ";
	print_move(top_move);
	cout << endl;
	cout << "eval: ";
	if(abs(top_score) >= CHECKMATE){
		if(top_score >= CHECKMATE){
			cout << "#" << to_move_sign*(depth+1)/2 << endl;
		} else {
			cout << "#" << -to_move_sign*(depth+1)/2 << endl;
		}
	} else {
		cout << to_move_sign*float(top_score)/100.0 << endl;
	}
	cout << "total time: " <<  tt;

	rootpos.add_move(top_move);
	rootpos.generate_moves();
	rootpos.print_pos(true);
	if(rootpos.to_move == WHITE){
		cout << "WHITE to move" << endl;
	} else {
		cout << "BLACK to move" << endl;
	}
	cout << endl << "next move: ";
	move = 0;

	while(move == 0){
		cin >> move_string;
		move = rootpos.pos_move_list.get_move_from_string(move_string);
	}
	print_move(move);
	rootpos.add_move(move);
	
	}

	// while(rootpos.pop_and_add()){
	// 	rootpos.next->generate_moves();
	// 	cout << rootpos.next->get_num_moves() << endl;
	// }

	return 0;
}