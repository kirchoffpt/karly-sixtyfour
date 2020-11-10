#include <memory>
#include <vector>
#include "chess_pos.h"
#include "constants.h"
#include "gtest/gtest.h"

//these tests are not exhaustive

class ChessPosFixture: public ::testing::Test { 
protected: 

    std::unique_ptr<chess_pos> ml = std::unique_ptr<chess_pos>(new chess_pos(STARTPOS));

    ChessPosFixture( ){ 
    } 

    void SetUp( ) { 
        
    }

    void TearDown( ) { 

    }

    ~ChessPosFixture( )  { 

    }

};

TEST_F(ChessPosFixture, PopAnyTarget) {
    EXPECT_EQ(0,0);
}







