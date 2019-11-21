#include "search.h"

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

#define MAX_AB_DEPTH 24
#define MIN_DEPTH 3
#define MAX_Q_DEPTH 24
#define MAX_DEPTH MAX_AB_DEPTH+MAX_Q_DEPTH
#define TABLE_SIZE 200000000

#define PVS_SEARCH TRUE // experimental. as opposed to regular minimax. this option will probably be removed in the future

using namespace std;

search_handler::search_handler(chess_pos* pos){
	rootpos = pos;
	rootpos->id = 0;
	search_id = 0;
	pv_moves.reserve(MAX_DEPTH);
	TT = new ttable;
	TT->resize(TABLE_SIZE);
	reset();
	return;
}

void search_handler::reset(){
	std::memset(&uci_s, 0, sizeof(search_options));
	past_positions.clear();
	best_move = 0;
	ponder_move = 0;
	nodes_searched = 0;
	depth_searched = 0;
	is_searching = FALSE;
	return;	
}

void search_handler::go(){
	if(is_searching) return;
	is_searching = TRUE;
	search_id++;
	TT->tt.clear();
	TT->resize(TABLE_SIZE);
	thread search_thread(&search_handler::search,this);
	search_thread.detach();
	return;	
}

void search_handler::max_timer(int ms, int id){
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	stop(id);
}

bool search_handler::is_threefold_repetition(const z_key position){
	int i, count = 1;
	for(i=0;i<past_positions.size();i++){
		if(past_positions[i] == position) count++;
	}
	if(count >= 3) return true;
	return false;
}

void search_handler::search(){
	
	int i,j,n_root_moves;
	int to_move_sign;
	int score, top_score, alpha, beta, depth, sel_depth;
	chess_pos* node_ptrs[MAX_DEPTH+1];
	unsigned short move, top_move;
	int move_scores[MAX_MOVES_IN_POS] = {0};
	int to_move;
	string info_str;
	long long k = 0;
	float t, tt;
	char move_string[5];
	clock_t cl;     //initializing a clock type
	float target_time, max_time, t1, t2;
	std::chrono::steady_clock::time_point start, end;

	i = MAX_DEPTH;
	node_ptrs[i--] = new chess_pos;
	for(i;i>=0;i--){
		node_ptrs[i] = new chess_pos;
		node_ptrs[i]->next = node_ptrs[i+1];
	}
	rootpos->next = node_ptrs[0];
	node_ptrs[0]->prev = rootpos;
	for(i=1;i<=MAX_DEPTH;i++){
		node_ptrs[i]->prev = node_ptrs[i-1];
	}
	for(i=0;i<MAX_DEPTH+1;i++){
		node_ptrs[i]->id = i+1;
	}

	rootpos->generate_moves();
	n_root_moves = rootpos->get_num_moves();

	if(n_root_moves == 1){
		best_move = rootpos->pos_move_list.pop_move();
		stop(search_id);
	}

	to_move = rootpos->to_move;
	to_move_sign = ((!to_move)*2-1);

	t1 = float(uci_s.time[to_move]);
	t2 = float(uci_s.time[!to_move]);
	target_time = max(t1/128 , t1 - 1.1*t2);

	if(uci_s.movetime > 0){
		max_time = uci_s.movetime;
	} else {
		max_time = t1 - 0.98*t2;
		if(max_time < 0) max_time = t1/64;
	}

	best_move = rootpos->pos_move_list.get_random_move();
	thread timer_thread(&search_handler::max_timer,this,int(max_time),search_id);
	timer_thread.detach();

    tt = 0;
    nodes_searched = 0;
	for(depth=min(MAX_AB_DEPTH,1);depth <= MAX_AB_DEPTH;depth++){
		top_score = SCORE_LO;
		alpha = SCORE_LO;
		beta =  SCORE_HI;
		cout << "d " +  to_string(depth) + "/" + to_string(MIN_DEPTH) + "\n";
		for(i=n_root_moves-1;i>=0;i--){
			if(tt > target_time && depth > MIN_DEPTH && top_score > 0) goto exit_minimax_loop;
			move = rootpos->pos_move_list.get_move(i);
			//if((58<<SRC_SHIFT) + (50<<DST_SHIFT) != move) continue;
			rootpos->next->copy_pos(rootpos);
			rootpos->next->add_move(move);
			if(is_threefold_repetition(rootpos->zobrist_key)){
				score = 0;
				nodes_searched++;
			} else {
				k = nodes_searched;
				start = std::chrono::steady_clock::now();
				if(PVS_SEARCH){ 
					score = -pvs(rootpos->next, depth-1,-to_move_sign, -beta, -alpha);
					alpha = max(alpha, score);
					score *= to_move_sign;
				} else {
					score = minimax(rootpos->next, depth-1,!rootpos->to_move, alpha, beta);
					if(rootpos->to_move){
						beta = min(beta, score);
					} else {
						alpha = max(alpha, score);
					}
				}
				end = std::chrono::steady_clock::now();
				t = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				tt += t;
			}
			move_scores[i] = score*to_move_sign;
			
			if(score*to_move_sign > top_score){
				top_move = move;
				top_score = score*to_move_sign;
				best_move = top_move;
			}

			if(!is_searching) return; 

			for(j=1;j<MAX_DEPTH;j++){
				if(node_ptrs[j]->occ[0] == 0){
					sel_depth = j-1;
					break;
				}
			}
			if(depth >= 4){
				info_str = "info currmove " + move_itos(move);
				info_str += " score cp " + to_string(top_score);
				info_str += " nodes " + to_string(nodes_searched);
				info_str +=  " depth " + to_string(depth);
				//info_str +=  " seldepth " + to_string(sel_depth);
				if(tt > 50){
				info_str += " time " + to_string(int(tt));
				info_str +=  " nps " + to_string(1000*int(nodes_searched/tt));
				//info_str += " hashfull " + to_string(TT->hashfull());
				}
				cout << info_str + "\n";
			}

			fflush(stdout);

			if(top_score >= CHECKMATE){
				goto exit_minimax_loop;
			}
		}
		if(top_score <= -CHECKMATE){
			break;
		}
		rootpos->pos_move_list.sort_moves_by_scores(move_scores);
	}

	exit_minimax_loop:

	// cout << endl;
	// cout << "top_move: ";
	// print_move(top_move);
	// cout << endl;
	// cout << "eval: ";
	if(abs(top_score) >= CHECKMATE){
		if(top_score >= CHECKMATE){
			// cout << "#" << to_move_sign*(depth+1)/2 << endl;
		} else {
			// cout << "#" << -to_move_sign*(depth+1)/2 << endl;
		}
	} else {
		// cout << to_move_sign*float(top_score)/100.0 << endl;
	}
	// cout << "total time: " <<  tt << endl;

	for(i=0;i<=MAX_AB_DEPTH+MAX_Q_DEPTH;i++){
		delete node_ptrs[i];
	}

	best_move = top_move;
	if(is_searching) stop(search_id);
	return;
}


void search_handler::stop(int id){
	if(!is_searching || search_id != id) return;
	cout << "bestmove " + move_itos(best_move) + "\n";
	fflush(stdout);
	is_searching = FALSE;
	return;
}

int search_handler::quiesce(chess_pos* node, int min_or_max, int a, int b, int depth, int last_eval, int last_delta){
	nodes_searched++;

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

int search_handler::minimax(chess_pos* node, int depth, int min_or_max, int a, int b){
	nodes_searched++;
	__assume(is_searching);
	if(!is_searching) return 0; 
	node->generate_moves();
	if(depth == 0){
		if(node->captures > 0){
			/////
			//node->print_pos(true);
			return quiesce(node,min_or_max,a,b,MAX_Q_DEPTH,node->prev->evaluation,SCORE_LO);
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

int search_handler::pvs(chess_pos* node, int depth, int color, int a, int b){
	nodes_searched++;
	__assume(is_searching);
	if(!is_searching) return 0;

	int eval = SCORE_LO;
	int a_cpy = a;
	unsigned short move, b_move;
	b_move = TT->find(node->zobrist_key, &eval, &a, &b, depth);
	if(eval != SCORE_LO) return eval; 

    node->generate_moves();
    if(b_move) node->pos_move_list.move_to_front(b_move);

	if(depth == 0){
		if(node->captures > 0){
			if(color == -1){
				return-quiesce(node,BLACK,-b,-a,MAX_Q_DEPTH,node->prev->evaluation,SCORE_LO);
			} else {
				return quiesce(node,WHITE,a,b,MAX_Q_DEPTH,node->prev->evaluation,SCORE_LO);
			}
		}
		return color*node->eval();
	}
	if(b_move = node->pop_and_add()){
		eval = -pvs(node->next, depth - 1, -color, -b,-a);
		a = max(a, eval);
        if(a >= b) goto done;
	} else {
		return color*node->mate_eval();
	}
    while(move = node->pop_and_add()){
    	eval = -pvs(node->next, depth - 1, -color, -a - 1, -a);
        if((a < eval) && (eval < b)) eval = -pvs(node->next, depth - 1, -color, -b, -eval);
        if(eval >= a){
        	a = eval;
        	b_move = move;
        }
        if(a >= b) break;
    }

    done:

    tt_entry entry;
    entry.depth = depth;
    entry.full_key = node->zobrist_key;
    entry.score = a;
    if(a <= a_cpy){
    	entry.node_type = ALLNODE;
    	entry.best_move = b_move;
    } else if(a >= b){
    	entry.node_type = CUTNODE;
    	entry.best_move = b_move;
    } else {
    	entry.node_type = PVNODE;
    	entry.best_move = b_move;
    }
    TT->place(node->zobrist_key, entry);

    return a;
}
