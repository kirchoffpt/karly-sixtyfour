#include "chess_pos.h"
#include "constants.h"
#include "search.h"
#include <sstream>

namespace uci{
    void uci_position(std::istringstream& is, chess_pos* rootpos, search_handler* searcher);
    void uci_go(std::istringstream& is, search_handler* searcher);
    void init(int argc, char *argv[]);
};


