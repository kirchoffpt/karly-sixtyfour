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
	string token, s, fenstring = STARTPOS;
	char move_string[5];
	unsigned short move = 0;

	is >> token;
	if(token == "startpos"){
		fenstring = STARTPOS;
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
	rootpos->load_new_fen(fenstring);
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
		}
		 else if(token == "movetime"){
			is >> i;
			uci_s.movetime = i;
		}
	}

	searcher->uci_s = uci_s;
	searcher->go();

	return;
}


int main(int argc, char *argv[]){
	string cmd, token;
	ofstream ofs;
	chess_pos *rootpos = new chess_pos(STARTPOS);
	search_handler *searcher = new search_handler(rootpos);
	time_t system_time = chrono::system_clock::to_time_t(chrono::system_clock::now());

	rootpos->generate_moves();

	if(DEBUG){
		ofs = ofstream(FILEOUT, ofstream::app);
		ofs << endl << ctime(&system_time) << endl;
	}

	while(getline(cin,cmd)){
		istringstream is(cmd);
		ofs << cmd << endl;
		is >> token;
		if(token == "uci"){
			cout << "id name karly64 " << VERSION << "\n";
			cout << "id author Paul Kirchoff\n";
			cout << "uciok\n";
		} else if(token == "isready"){
			cout << "readyok\n";
		} else if(token == "position"){
			uci_position(is, rootpos, searcher);
		} else if(token == "go"){
			uci_go(is, searcher);
		} else if(token == "stop"){
			searcher->stop(searcher->search_id);
		} else if(token == "quit"){
			break;
		}
	}

	ofs.close();
	return 0;
}