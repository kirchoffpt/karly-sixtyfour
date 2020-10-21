#include "search.h"

#define TABLE_SIZE 256E6
#define LMR_LIMIT 3
#define LMR_DEPTH_REDUCTION 2
#define Q_SEARCH_DEPTH search_depth

using namespace std;

search_handler::search_handler(chess_pos* pos){
	rootpos = pos;
	rootpos->ply = 0;
	search_id = 0; //incrememt before searching
	principal_variation.reserve(MAX_DEPTH);
	TT = new ttable;
	TT->resize(TABLE_SIZE);
	reset();
	return;
}

search_handler::~search_handler(){
	delete TT;
	return;
}

void search_handler::reset(){
	is_searching = false;
	std::memset(&uci_s, 0, sizeof(search_options));
	past_positions.clear();
	search_id = 0;
	best_move = 0;
	ponder_move = 0;
	TT->tt.clear();
	TT->resize(TABLE_SIZE);
	return;	
}

void search_handler::go(){
	if(is_searching) return;
	is_searching = true;
	search_id++;
	overall_top_score = SCORE_LO;
	if(CLEAR_TTABLE_BEFORE_SEARCH){
		TT->tt.clear();
		TT->resize(TABLE_SIZE);
	}
	thread search_thread(&search_handler::search,this);
	search_thread.detach();
	return;	
}

void search_handler::max_timer(int ms, float incr){
	std::chrono::steady_clock::time_point start, end;

	do{
		start = std::chrono::steady_clock::now();
		end = std::chrono::steady_clock::now();

		//rather than simple timer use this loop to check if we are still searching every so often
		while(is_searching && 
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < ms){

			std::this_thread::sleep_for(std::chrono::microseconds(500));
			end = std::chrono::steady_clock::now();

		}

		//if we are in correspondence mode we may increase the time instead of finishing
		float depth_score = (overall_top_score+CSPOND_CONTEMPT)*powf(1.5,search_depth);
		if( depth_score < CSPOND_TRGT_DEPTHSCORE){
			ms = incr;
			incr *= CSPOND_TIME_DECAY;
		} else {
			ms = 0;
		}
	} while (is_searching && ms > 100);
	

	stop();
	return;
}

int search_handler::num_repetitions(const z_key position){
	size_t i, count = 0;
	for(i=0;i<past_positions.size();i++){
		if(past_positions[i] == position) count++;
	}
	return count;
}

bool search_handler::allows_threefold(const chess_pos* c1){
	unsigned short move; 
	tt_entry entry = {0};
	chess_pos p1, p2;
	p1 = *const_cast<chess_pos*>(c1);
	p1.next = &p2;
	p1.generate_moves();
	while( (move = p1.pop_and_add()) ){
		if (num_repetitions(p2.zobrist_key) >= 2){
			entry.age = search_id;
			entry.node_type = PVNODE;
			entry.depth = MAX_DEPTH;
			entry.score = 0;
			TT->place(p2.zobrist_key, entry);
			//cout << "info string " << move_itos(move) << " will draw after " << move_itos(*p1-*rootpos) << endl;
			break;
		}
	}
	return is_threefold(c1);
}

bool search_handler::is_threefold(const chess_pos* c1){
	tt_entry entry = {0};
	if (num_repetitions(c1->zobrist_key) >= 2){
		entry.age = search_id;
		entry.node_type = PVNODE;
		entry.depth = MAX_DEPTH;
		entry.score = 0;
		TT->place(c1->zobrist_key, entry);
		return true;
	}
	return false;
}

void search_handler::search(){
	
	int i,n_root_moves;
	int to_move_sign;
	int top_score, score, alpha, beta;
	chess_pos* node_ptrs[MAX_DEPTH+1];
	unsigned short move;
	int move_scores[MAX_MOVES_IN_POS] = {0};
	int to_move;
	string info_str;
	float t, total_time;
	float max_time, t1, t2, cspond_time_incr;
	bool timed;
	std::chrono::steady_clock::time_point start, end;

	i = MAX_DEPTH;
	node_ptrs[i--] = new chess_pos;
	for(;i>=0;i--){
		node_ptrs[i] = new chess_pos;
		node_ptrs[i]->next = node_ptrs[i+1];
	}
	rootpos->next = node_ptrs[0];
	node_ptrs[0]->prev = rootpos;
	for(i=1;i<=MAX_DEPTH;i++){
		node_ptrs[i]->prev = node_ptrs[i-1];
	}
	for(i=0;i<MAX_DEPTH+1;i++){
		node_ptrs[i]->ply = i+1;
	}

	rootpos->generate_moves();
	n_root_moves = rootpos->get_num_moves();

	to_move = rootpos->to_move;
	to_move_sign = ((!to_move)*2-1);

	//set up time variables

	t1 = float(uci_s.time[to_move]);
	t2 = float(uci_s.time[!to_move]);

	if(t1 >= CORRESPONDENCE_MODE_THRESHOLD){

		cout << "\nCORRESPONDENCE SEARCH\n";
		cspond_time_incr = CSPOND_TIME_INCREMENT;
		max_time = CSPOND_TIME_BASE;

	} else {
		cspond_time_incr = 0;

		if(uci_s.movetime){
			max_time = uci_s.movetime;
		} else {
			max_time = (0.99*t1/(t2+1)+t1*powf(0.7,0.022+12*t2/t1))/2+t1/128;
			if(uci_s.inc[to_move]) max_time = min(t1,max_time+uci_s.inc[to_move]);
			max_time = min((double)max_time,t1*MAX_MOVE_TIME_USAGE);
			max_time = max(100.0, (double)max_time);
		}

	}

	if(t1 <= 0 && uci_s.movetime <= 0){
		max_time = F_HI - 1;
		timed = false;
	} else {
		thread timer_thread(&search_handler::max_timer,this,int(max_time),cspond_time_incr);
		timer_thread.detach();
		timed = true;
	}


	//begin search loop
	//if only one move available just make it, skip output
	if(n_root_moves == 1){
		best_move = rootpos->pos_move_list.pop_move();
		goto exit_search;
	}

	best_move = rootpos->pos_move_list.get_random_move();
    total_time = 1;
    nodes_searched = 0;

	for(search_depth=min(MAX_AB_DEPTH,1);search_depth <= MAX_AB_DEPTH;search_depth++){
		top_score = SCORE_LO;
		alpha = SCORE_LO;
		beta =  SCORE_HI;
		for(i=n_root_moves-1;i>=0;i--){
			move = rootpos->pos_move_list.get_move(i);
			//if((14<<SRC_SHIFT) + (6<<DST_SHIFT) != move) continue;
			rootpos->next->copy_pos(rootpos);
			rootpos->next->add_move(move);
			if(allows_threefold(rootpos->next)){
				score = 0;
				nodes_searched++;
			} else {
				start = std::chrono::steady_clock::now();

				if(i==n_root_moves-1){
					score = -pvs(rootpos->next, search_depth-1,-to_move_sign, -beta, -alpha);
				} else {
					score = -pvs(rootpos->next, search_depth-1,-to_move_sign, -alpha-1, -alpha);
		        	if((alpha < score) && (score < beta)) score = -pvs(rootpos->next, search_depth-1,-to_move_sign, -beta, -score);
	        	}
				if(search_depth > 2) alpha = max(alpha, score);

				end = std::chrono::steady_clock::now();
				t = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				total_time += t;
			}
			move_scores[i] = score;
			
			if(score > top_score){
				top_score = score;
				overall_top_score = max(overall_top_score, top_score);
				best_move = move;
			}

			if(!is_searching) goto exit_search; 

			if(total_time > 10 && (move == best_move || score != top_score)){
				info_str = "info";
				info_str +=  " depth " + to_string(search_depth);
				info_str += " currmove " + move_itos(move);
				//info_str += " currmovenumber " + to_string(n_root_moves-i);
				if(abs(score) >= CHECKMATE-MATE_BUFFER){
					info_str += " score mate " + to_string(int((abs(CHECKMATE)-abs(score)+1)/2*(abs(score)/score)));
				} else {
					info_str += " score cp " + to_string(score);
				}
				info_str += " nodes " + to_string(nodes_searched);
				info_str +=  " nps " + to_string(1000*int(nodes_searched/total_time));
				info_str += " time " + to_string(int(total_time));
				//info_str += " hashfull " + to_string(TT->hashfull());
				if(is_searching) cout << info_str + "\n";
			}

			fflush(stdout);

			if(timed && top_score >= CHECKMATE-MATE_BUFFER){
				break;
			}
		}
		info_str = "info";
		info_str +=  " depth " + to_string(search_depth);
		info_str += " seldepth " + to_string(rootpos->clear_next_occs()-search_depth);
		if(abs(top_score) >= CHECKMATE-MATE_BUFFER){
			info_str += " score mate " + to_string(int((abs(CHECKMATE)-abs(top_score)+1)/2*(abs(top_score)/top_score)));
		} else {
			info_str += " score cp " + to_string(top_score);
		}
		info_str += " nodes " + to_string(nodes_searched);
		info_str +=  " nps " + to_string(1000*int(nodes_searched/total_time));
		info_str += " time " + to_string(int(total_time));
		//info_str += " hashfull " + to_string(TT->hashfull());
		info_str += " pv " + TT->extract_pv(rootpos, best_move);
		cout << info_str + "\n";
		if(timed && top_score >= CHECKMATE-MATE_BUFFER){
			break;
		}
		fflush(stdout);
		rootpos->pos_move_list.sort_moves_by_scores(move_scores);
		if(uci_s.depth_limit && (search_depth >= uci_s.depth_limit)) goto exit_search;
	}

	exit_search:

	for(i=0;i<=MAX_DEPTH;i++){
		delete node_ptrs[i];
	}

	if(is_searching && !uci_s.movetime) stop();
	return;
}


void search_handler::stop(){
	if(!is_searching) return;

	cout << "bestmove " + move_itos(best_move) + "\n";
	fflush(stdout);

	cout << "PV " + TT->extract_pv(rootpos, best_move) + "\n";
	fflush(stdout);


	is_searching = false;
	return;
}

int search_handler::quiesce(chess_pos* node, int depth, int color, int a, int b){
	if(!is_searching) return 0;

	int eval;

	if(depth < Q_SEARCH_DEPTH){ //dont regenerate moves if we just entered q search
		node->generate_moves(); //must generate moves for eval;
		nodes_searched++;
	}

	if(node->get_num_moves() <= 0){
		return color*node->mate_eval();
	}

	eval = color*node->eval(); //stand pat score "do nothing eval"
	if(node->captures == 0 || depth == 0) return eval; 
	if(eval >= b) return eval;
	a = max(a, eval);

	node->order_moves();
	
	while(node->pop_and_add_capture() > 1){
		eval = -quiesce(node->next, depth-1, -color, -b, -a);
		a = max(a, eval);
		if(a >= b) break;
	}

    return a;
}

int search_handler::pvs(chess_pos* node, int depth, int color, int a, int b){
	if(!is_searching) return 0;

	int eval = SCORE_LO;
	int a_cpy = a;
	unsigned short move, b_move;
	int moves_searched, noreduce_moves = MAX_MOVES_IN_POS;
	tt_entry entry;
	chess_pos* past_node = node;

	nodes_searched++;

	//check for repeated position
	unsigned long long this_key = node->zobrist_key;
	while(past_node->ply > 1){
		past_node = past_node->prev->prev; //go back two positions each time
		if(past_node->zobrist_key == this_key){
			return 0;
		}
	}

	node->generate_moves(); //must generate moves for eval;

	if((node->in_check) && (depth < 3)) depth++;

    //enter quiescence search at horizon nodes
	if(depth <= 0){
		if(node->captures > 0){
			return quiesce(node,Q_SEARCH_DEPTH,color,a,b);
		}
		return color*node->eval();
	}

	//transposition table lookup
	b_move = TT->find(node->zobrist_key, &eval, &a, &b, depth, search_id);
	if(eval != SCORE_LO) return eval;

	//forward null move pruning
	if( ENABLE_NULL_MOVE_PRUNING && (depth >= 4) && node->fwd_null_move()){
		eval = -pvs(node->next, depth/2-2, -color, -b,-b + 1);
		if(eval >= b) return eval;
		eval = SCORE_LO; //null move failed to produce a cut off
	}

	//move ordering
	if(depth > LMR_LIMIT){
		noreduce_moves = node->order_moves_smart();
	} else {
		node->order_moves();
	}
	if(b_move) node->pos_move_list.move_to_front(b_move);

	//principal variation search (first move)
	if( (b_move = node->pop_and_add()) ){
		eval = -pvs(node->next, depth - 1, -color, -b,-a);
		a = max(a, eval);
        if(a >= b) goto done;
	} else {
		return color*node->mate_eval();
	}
	moves_searched = 1;

	//null window searches (later moves)
    while( (move = node->pop_and_add()) ){
    	if((moves_searched++ >= noreduce_moves/2) && (depth > LMR_LIMIT)){
    		eval = -pvs(node->next, depth - 1 - LMR_DEPTH_REDUCTION, -color, -a - 1, -a);
    	} else {
    		eval = a + 1;
    	}
    	if(eval > a){
	    	eval = -pvs(node->next, depth - 1, -color, -a - 1, -a);
	        if((a < eval) && (eval < b)) eval = -pvs(node->next, depth - 1, -color, -b, -eval);
    	}
        if(eval > a){
        	a = eval;
        	b_move = move;
        }
        if(a >= b) break;
    }

    done:

    //entry into transposition tabl-
    entry.depth = depth;
    entry.score = a;
    entry.age = search_id;
    entry.best_move = b_move;
    if(a <= a_cpy){
    	entry.node_type = ALLNODE;
    } else if(a >= b){
    	entry.node_type = CUTNODE;
    } else {
    	entry.node_type = PVNODE;
    }
    TT->place(this_key, entry);

    return a;
}
