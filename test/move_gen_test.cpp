#include <memory>
#include <vector>
#include "chess_pos.h"
#include "constants.h"
#include "gtest/gtest.h"

class MoveGenerationTest: public ::testing::Test { 
protected: 

    chess_pos rpos, rpos2, rpos3;

    MoveGenerationTest( ){ 
    } 

    void SetUp( ) { 
        rpos = chess_pos(STARTPOS);
        rpos.generate_moves();
        rpos.next = &rpos2;
        rpos2.next = &rpos3;
    }

    void TearDown( ) { 

    }

    ~MoveGenerationTest( )  { 

    }

};

TEST_F(MoveGenerationTest, Position1) {

}






