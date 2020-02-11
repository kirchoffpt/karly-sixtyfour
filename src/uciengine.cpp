#include "chess_pos.h"
#include "constants.h"
#include "search.h"
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>   

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
#define FILEOUT "uci_input_log.txt"

using namespace std;

void uci_position(istringstream& is, chess_pos* rootpos, search_handler* searcher){
	string token, s, fenstring;
	char move_string[5];
	unsigned short move = 0;

	while(searcher->is_searching){
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if(searcher->is_searching){
			//search handler is clearly still searching do not load another position
			return;
		}
	}

	is >> token;
	if(token == "startpos"){
		fenstring = STARTPOS;
	} else if(token == "pos"){
		fenstring = "\n";
	} else if(token == "fen"){
		is >> token;
		while(is >> s && s != "moves"){
			token.append(" "+s);
		}
		fenstring = token;
	} else {
		return;
	}
	searcher->past_positions.clear();
	if(fenstring != "\n") rootpos->load_new_fen(fenstring);
	rootpos->generate_moves();
	rootpos->sort_piece_list();
	searcher->past_positions.push_back(rootpos->zobrist_key);
	//cout << token << endl;
	if(s == "moves" || (is >> token && token == "moves")){
		while(is >> token){
			strcpy(move_string, token.c_str());
			move = rootpos->pos_move_list.get_move_from_string(move_string);
			if(move != 0){
				rootpos->add_move(move);
				rootpos->generate_moves();
				rootpos->sort_piece_list();
				searcher->past_positions.push_back(rootpos->zobrist_key);
			}
		}
	}
	//rootpos->sort_piece_list();
	//rootpos->print_pos(true);
}

void uci_go(istringstream& is, search_handler* searcher){
	string token;
	int i;
	search_options uci_s = {0};

	while(is >> token){
		if(token == "wtime"){
			is >> i;
			uci_s.time[WHITE] = i;
		} else if(token == "btime"){
			is >> i;
			uci_s.time[BLACK] = i;
		} else if(token == "winc"){
			is >> i;
			uci_s.inc[WHITE] = i;
		} else if(token == "binc"){
			is >> i;
			uci_s.inc[BLACK] = i;
		} else if(token == "movetime"){
			is >> i;
			uci_s.movetime = i;
		} else if(token == "depth"){
			is >> i;
			uci_s.depth_limit = i;
		} else if(token == "nodes"){
			is >> i;
			uci_s.nodes_limit = i;
		}
	}

	searcher->uci_s = uci_s;
	searcher->go();

	return;
}


int main(int argc, char *argv[]){
	string cmd, token;
	ofstream ofs;
	chess_pos *rootpos;
	search_handler *searcher;
	time_t system_time = chrono::system_clock::to_time_t(chrono::system_clock::now());

	if(argc > 1) token = argv[1];
	else token = STARTPOS;

	rootpos = new chess_pos(token); 
	searcher = new search_handler(rootpos);
	rootpos->generate_moves();

	if(LOG_UCI_INPUT){
		ofs = ofstream(FILEOUT, ofstream::app);
		ofs << endl << ctime(&system_time) << endl;
	}

	while(getline(cin,cmd)){
		istringstream is(cmd);
		ofs << cmd << endl;
		if(!(is >> token)) continue;
		if(token == "uci"){
			cout << "id name karly64 " << VERSION << "\n";
			cout << "id author Paul Kirchoff\n";
			cout << "uciok\n";
		} else if(token == "isready"){
			cout << "readyok\n";
		} else if(token == "position" || token == "pos"){
			uci_position(is, rootpos, searcher);
		} else if(token == "go"){
			uci_go(is, searcher);
		} else if(token == "stop"){
			searcher->stop();
		} else if(token == "ucinewgame" || token == "reset"){
			searcher->reset();
		} else if(token == "showpos"){
			rootpos->print_pos(false);
		} else if(token == "quit"){
			searcher->stop();
			break;
		} else if(token == "help"){
			cout << "position [fen <fenstring> | startpos | pos]  moves <move1> .... <movei>" << endl;
			cout << "go wtime <x> btime <y> depth <d> movetime <t>" << endl;
			cout << "uci isready stop ucinewgame showpos quit" << endl;
		}
	}

	delete rootpos;
	delete searcher;
	ofs.close();
	return 0;
}