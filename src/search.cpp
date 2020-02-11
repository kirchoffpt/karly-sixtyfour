#include "search.h"


#define TABLE_SIZE 256E6

using namespace std;

search_handler::search_handler(chess_pos* pos){
	rootpos = pos;
	rootpos->id = 0;
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
	std::memset(&uci_s, 0, sizeof(search_options));
	past_positions.clear();
	search_id = 0;
	best_move = 0;
	ponder_move = 0;
	TT->tt.clear();
	TT->resize(TABLE_SIZE);
	is_searching = FALSE;
	return;	
}

void search_handler::go(){
	if(is_searching) return;
	is_searching = TRUE;
	search_id++;
	if(CLEAR_TTABLE_BEFORE_SEARCH){
		TT->tt.clear();
		TT->resize(TABLE_SIZE);
	}
	thread search_thread(&search_handler::search,this);
	search_thread.detach();
	return;	
}

void search_handler::max_timer(int ms){
	std::chrono::steady_clock::time_point start, end;
	start = std::chrono::steady_clock::now();
	end = std::chrono::steady_clock::now();
	while(is_searching && 
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < ms){
		std::this_thread::sleep_for(std::chrono::microseconds(100));
		end = std::chrono::steady_clock::now();
	}
	stop();
	return;
}

int search_handler::num_repetitions(const z_key position){
	int i, count = 0;
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
	while(move = p1.pop_and_add()){
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
	
	int i,j,n_root_moves;
	int to_move_sign;
	int score, top_score, alpha, beta, sel_depth;
	chess_pos* node_ptrs[MAX_DEPTH+1];
	unsigned short move;
	int move_scores[MAX_MOVES_IN_POS] = {0};
	int to_move;
	string info_str;
	long long k = 0;
	float t, total_time;
	char move_string[5];
	clock_t cl;     //initializing a clock type
	float max_time, t1, t2;
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

	to_move = rootpos->to_move;
	to_move_sign = ((!to_move)*2-1);

	t1 = float(uci_s.time[to_move]);
	t2 = float(uci_s.time[!to_move]);

	if(uci_s.movetime){
		max_time = uci_s.movetime;
	} else {
		max_time = (0.99*t1/(t2+1)+t1*powf(0.7,0.022+12*t2/t1))/2+t1/128;
		if(uci_s.inc[to_move]) max_time = min(t1,max_time+uci_s.inc[to_move]);
		max_time = min(max_time,t1*MAX_MOVE_TIME_USAGE);
		max_time = max(100, max_time);
	}

	if(t1 <= 0 && uci_s.movetime <= 0){
		max_time = FLT_MAX - 1;
	} else {
		thread timer_thread(&search_handler::max_timer,this,int(max_time));
		timer_thread.detach();
	}

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
		cout << "\nD" << search_depth << endl;
		for(i=n_root_moves-1;i>=0;i--){
			move = rootpos->pos_move_list.get_move(i);
			//if((14<<SRC_SHIFT) + (6<<DST_SHIFT) != move) continue;
			rootpos->next->copy_pos(rootpos);
			rootpos->next->add_move(move);
			if(allows_threefold(rootpos->next)){
				score = 0;
				nodes_searched++;
			} else {
				k = nodes_searched;
				start = std::chrono::steady_clock::now();

				score = -pvs(rootpos->next, search_depth-1,-to_move_sign, -beta, -alpha);
				alpha = max(alpha, score);

				end = std::chrono::steady_clock::now();
				t = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				total_time += t;
			}
			move_scores[i] = score;
			
			if(score > top_score){
				top_score = score;
				best_move = move;
			}

			if(!is_searching) goto exit_search; 

			if(total_time > 10){
				info_str = "info";
				info_str +=  " depth " + to_string(search_depth);
				info_str += " currmove " + move_itos(move);
				//info_str += " currmovenumber " + to_string(n_root_moves-i);
				info_str += " score cp " + to_string(top_score);
				info_str += " nodes " + to_string(nodes_searched);
				info_str +=  " nps " + to_string(1000*int(nodes_searched/total_time));
				info_str += " time " + to_string(int(total_time));
				//info_str += " hashfull " + to_string(TT->hashfull());
				if(is_searching) cout << info_str + "\n";
			}

			fflush(stdout);

			if(top_score >= CHECKMATE){
				goto exit_search;
			}
		}
		info_str = "info";
		info_str +=  " depth " + to_string(search_depth);
		info_str += " score cp " + to_string(top_score);
		info_str += " nodes " + to_string(nodes_searched);
		info_str +=  " nps " + to_string(1000*int(nodes_searched/total_time));
		info_str += " time " + to_string(int(total_time));
		//info_str += " hashfull " + to_string(TT->hashfull());
		info_str += " pv " + TT->extract_pv(rootpos, best_move);
		cout << info_str + "\n";
		if(top_score <= -CHECKMATE){
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


	is_searching = FALSE;
	return;
}

int search_handler::quiesce(chess_pos* node, int depth, int color, int a, int b, bool gen_moves){

	int eval;

	if(gen_moves){
		node->generate_moves();
		nodes_searched++;
	}

	if(node->get_num_moves() <= 0){
		return color*node->mate_eval();
	}

	eval = color*node->eval(); //stand pat score "do nothing eval"
	if(node->captures == 0 || depth == 0) return eval; 
	if(eval >= b) return b;
	a = max(a, eval);

	node->order_moves();
	
	while(node->pop_and_add_capture() > 1){
		eval = -quiesce(node->next, depth-1, -color, -b, -a, TRUE);
		a = max(a, eval);
		if(a >= b) break;
	}

    return a;
}

int search_handler::pvs(chess_pos* node, int depth, int color, int a, int b){
	__assume(is_searching);
	if(!is_searching) return 0;

	int eval = SCORE_LO;
	int a_cpy = a;
	unsigned short move, b_move;
	tt_entry entry;

    node->generate_moves(); //must generate moves for eval;
    nodes_searched++;

    //enter quiescence search at horizon nodes
	if(depth <= 0){
		if(node->captures > 0){
			return quiesce(node,MAX_Q_DEPTH,color,a,b,FALSE);
		}
		return color*node->eval();
	}

	//forward null move pruning
	if( ENABLE_NULL_MOVE_PRUNING && (depth >= 4) && node->fwd_null_move()){
		eval = -pvs(node->next, depth/2-2, -color, -b,-b + 1);
		if(eval >= b) return b;
		eval = SCORE_LO;
	}

	//transposition table lookup
	b_move = TT->find(node->zobrist_key, &eval, &a, &b, depth, search_id);
	if(eval != SCORE_LO) return eval;
	node->order_moves();
	if(b_move) node->pos_move_list.move_to_front(b_move);

	//principal variation search (first move)
	if(b_move = node->pop_and_add()){
		eval = -pvs(node->next, depth - 1, -color, -b,-a);
		a = max(a, eval);
        if(a >= b) goto done;
	} else {
		return color*node->mate_eval();
	}

	//null window searches (later moves)
    while(move = node->pop_and_add()){
    	eval = -pvs(node->next, depth - 1, -color, -a - 1, -a);
        if((a < eval) && (eval < b)) eval = -pvs(node->next, depth - 1, -color, -b, -eval);
        if(eval > a){
        	a = eval;
        	b_move = move;
        }
        if(a >= b) break;
    }

    done:

    //entry into transposition table
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
    TT->place(node->zobrist_key, entry);

    return a;
}
