#include "chess_pos.h"
#include "constants.h"
#include "search.h"
#include <sstream>
#include <map>

struct uci_option{
        std::string option_name;
        std::string option_type;
        std::string option_default;
        std::string option_min;
        std::string option_max;
        std::vector<std::string> option_var;
        uci_option(std::string n, std::string t, std::string d, std::string min, std::string max, std::vector<std::string> var){
            option_name = n;
            option_type = t;
            option_default = d;
            option_min = min; 
            option_max = max;
            option_var = var;
        };
        uci_option(){}; //default constructor
};

class uci {
    std::map<std::string, uci_option> options;

    public:
    uci();
    void uci_position(std::istringstream& is, chess_pos* rootpos, search_handler* searcher);
    void uci_go(std::istringstream& is, search_handler* searcher);
    void uci_getoptions();
    void uci_setoption(std::istringstream& is, search_handler* searcher);
    void init(int argc, char *argv[]);
};


