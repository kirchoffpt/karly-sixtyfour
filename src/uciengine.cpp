#include "chess_pos.h"
#include "constants.h"
#include "search.h"
#include "k64config.h"
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>  
#include <cstring>
#include <thread>

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
#define FILEOUT "uci_input_log.txt"

using namespace std;

void uci_position(istringstream& is, chess_pos* rootpos, search_handler* searcher){
	string token, s, fenstring;
	uint16_t move = 0;

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
	if(s == "moves" || (is >> token && token == "moves")){
		while(is >> token){
			move = rootpos->pos_move_list.get_move_from_string(token);
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
	std::chrono::steady_clock::time_point start_t, curr_t;
	string version_str = to_string(karly64_VERSION_MAJOR) + "." + to_string(karly64_VERSION_MINOR) + "." + to_string(karly64_VERSION_PATCH);
	string build_str = CMAKE_BUILD_TYPE;

	start_t = std::chrono::steady_clock::now();

	if(argc > 1) token = argv[1];
	else token = STARTPOS;

	rootpos = new chess_pos(token);
	searcher = new search_handler(rootpos);
	rootpos->generate_moves();

	if(LOG_UCI_INPUT){
		ofs = ofstream(FILEOUT, ofstream::app);
		ofs << endl << ctime(&system_time) << endl;
	}

    cout <<	"KARLY64v" + version_str;
	if(build_str.length() > 0) cout << " Build: " + build_str;
	cout << endl;

	while(getline(cin,cmd)){
		istringstream is(cmd);
		curr_t = std::chrono::steady_clock::now();
		ofs << std::chrono::duration_cast<std::chrono::microseconds>(curr_t - start_t).count() << "->	"<< cmd << endl;
		if(!(is >> token)) continue;
		if(token == "uci"){
			cout << "id name karly64 " + version_str + "\n";
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
		} else if(token == "showmoves"){
			rootpos->pos_move_list.print_moves();
		} else if(token == "showfen"){
			cout << rootpos->get_fen() << endl;
		} else if(token == "hashfull"){
			cout << to_string(searcher->get_ttable()->hashfull()) + "%" << endl;
		} else if(token == "quit" || token == "exit"){
			searcher->stop();
			break;
		} else if(token == "help"){
			cout << "position [fen <fenstring> | startpos | pos]  moves <move1> .... <movei>" << endl;
			cout << "go wtime <x> btime <y> depth <d> movetime <t>" << endl;
			cout << "uci isready stop ucinewgame showpos showmoves showfen quit" << endl;
			cout << "*see standard uci protocol for more info on some of the above commands*" << endl;
		} else {
			cout << "type help for list of commands..." << endl;
		}
	}

	delete rootpos;
	delete searcher;
	ofs.close();
	return 0;
}