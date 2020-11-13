#include <memory>
#include <vector>
#include "postree.h"
#include "constants.h"
#include "gtest/gtest.h"

class MoveGenTest: public ::testing::Test { 
protected: 

    MoveGenTest( ){ 

    } 

    void SetUp( ) { 
        
    }

    void TearDown( ) { 

    }

    ~MoveGenTest( )  { 

    }

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

    long long run_perft(std::string fen, int depth){
        static long long total = 0;
        long long count = 0;
        chess_pos rpos(fen);
        postree tree(rpos, depth);
        perft(tree.root(),depth,&count);
        total += count;
        std::cout << total << std::endl;
        return count;
    }

};

class MoveGenTestP : 
    public MoveGenTest,
    public ::testing::WithParamInterface<std::tuple<std::string, int, int>> {
};

TEST_P(MoveGenTestP, Perft) {
    EXPECT_EQ(run_perft(std::get<0>(GetParam()) , std::get<1>(GetParam())), std::get<2>(GetParam()));
}

//from https://www.chessprogramming.org/Perft_Results
INSTANTIATE_TEST_CASE_P(
        MoveGenTest,
        MoveGenTestP,
        ::testing::Values(
                std::make_tuple(STARTPOS, 5, 4865609),
                std::make_tuple("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 4, 4085603),
                std::make_tuple("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 6, 11030083),
                std::make_tuple("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292),
                std::make_tuple("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", 5, 15833292),
                std::make_tuple("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ", 5, 89941194),
                std::make_tuple("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594),
                std::make_tuple("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 5, 3605103),
                std::make_tuple("5K2/8/8/1k6/2p4R/6r1/1P1P1B2/8 b - -", 5, 3342845),
                std::make_tuple("5K2/8/4b3/4k3/4p3/R6R/1PPP1PP1/8 w - -", 5, 9777730)
                ));




