#include "chess_pos.h"
#include <chrono>

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

#define FILENAME "../perftpositions.txt"

using namespace std;

void perft(chess_pos* node, int depth, long long* n){
	node->generate_moves();
	if(depth == 1){
		*n += node->get_num_moves();
		return;
	}
	while(node->pop_and_add()){
		perft(node->next, depth-1, n);
	}
	return;
}

int main(int argc, char *argv[]){
	int i,j;
	unsigned int x;
	chess_pos* rootpos, *node_ptrs[MAX_DEPTH];
	long long k = 0, total_nodes = 0;
	std::chrono::steady_clock::time_point start, end;
	int test_depth;
	float t = 0, dt = 0;
	int num_trials, num_failed = 0;
	long long test_nodes;
	bool passed = false;
	string test_fen, skip;
	ifstream infile (FILENAME, ifstream::in);

	if(argc > 1){
		num_trials = 1;
		test_depth = atoi(argv[1]);
		test_fen = argv[2];
	} else {
		infile >> num_trials;
	}

	i = MAX_DEPTH-1;
	node_ptrs[i--] = new chess_pos;
	for(;i>=0;i--){
		node_ptrs[i] = new chess_pos;
		node_ptrs[i]->next = node_ptrs[i+1];
	}

	cout << "trials: " << num_trials << endl;
	for(j=0;j<num_trials;j++){
		if(argc == 1){
			getline(getline(infile, skip, '"'), test_fen, '"');
			infile >> test_depth;
			infile >> test_nodes;
		}
		passed = false;
		rootpos = new chess_pos(test_fen);
		rootpos->next = node_ptrs[0];
		rootpos->generate_moves();
		if(argc > 1){
			for(x=0;x<rootpos->get_num_moves();x++){
				print_move(rootpos->pos_move_list.get_move(x));
				cout << endl;
			}
		}
		cout << "[   ] " + test_fen;
		fflush(stdout);
		for(i=1;i<=test_depth;i++){
			k = 0;
		    start = std::chrono::steady_clock::now();

			perft(rootpos, i, &k);

			end = std::chrono::steady_clock::now();

			dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/1000.0;

			t += dt;

			if(argc > 1){
				cout << "depth: " << i << "	nodes:" << k << "	time: " << dt;
				cout << endl;
			}

			total_nodes += k;
			if(i == test_depth && k == test_nodes) passed = true;
		}
		if(argc > 1) break;
		if(passed){
			cout << "\r[ \033[1;32m" + to_string(k) + "/" + to_string(test_nodes) + "\033[0m ] " + test_fen  << flush;
		} else {
			cout << "\r[ \033[1;31m" + to_string(k) + "/" + to_string(test_nodes) + "\033[0m ] " + test_fen  << flush;
			num_failed++;
		}
		cout << endl;
		delete rootpos;
	}

	cout << endl;
	if(argc == 1) cout << (num_trials-num_failed) << " / " << num_trials << " PASSED " << endl;

	cout << "time: " << t << endl;
	cout << "millions of moves per sec: " << float(total_nodes)/(1000000*t) << endl;

	infile.close();

	return 0;
}