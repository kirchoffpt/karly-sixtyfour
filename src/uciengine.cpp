#include "chess_pos.h"
#include "constants.h"
#include "search.h"
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>   

#define STARTPOS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
#define FILEOUT "uci_input_log.txt"
#define QUOTATION_MARK '"'

using namespace std;

void uci_position(istringstream& is, chess_pos* rootpos){
	string token, s, fenstring = STARTPOS;
	char move_string[5];
	unsigned short move = 0;

	is >> token;
	if(token == "startpos"){
		fenstring = STARTPOS;
	} else if(token == "fen"){
		is >> token;
		while(token.back() != QUOTATION_MARK && is){
			is >> s;
			token.append(" "+s);
		}
		fenstring = token;
	} else {
		return;
	}
	rootpos->load_new_fen(fenstring);
	rootpos->generate_moves();
	rootpos->sort_piece_list();
	//cout << token << endl;
	if(is >> token && token == "moves"){
		while(is >> token){
			strcpy(move_string, token.c_str());
			move = rootpos->pos_move_list.get_move_from_string(move_string);
			if(move != 0){
				rootpos->add_move(move);
				rootpos->generate_moves();
				rootpos->sort_piece_list();
			}
		}
	}
	//rootpos->sort_piece_list();
	//rootpos->print_pos(true);
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
			uci_position(is, rootpos);
		} else if(token == "go"){
			searcher->go();
		} else if(token == "stop"){
			searcher->stop();
		} else if(token == "quit"){
			break;
		}
	}

	ofs.close();
	return 0;
}