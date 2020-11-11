#include <memory>
#include <vector>
#include "ttable.h"
#include "constants.h"
#include "gtest/gtest.h"

#define TEST_T_SIZE 1E6
#define BEST_MOVE 42


class TTableF: public ::testing::Test { 
protected: 

    ttable t;
    tt_entry test_entry;

    TTableF( ){ 
    } 

    void SetUp( ) { 
        t = ttable();
        t.resize(TEST_T_SIZE);
        test_entry.full_key = 0;
        test_entry.score = 25;
        test_entry.age = 1;
        test_entry.depth = 1;
        test_entry.best_move = BEST_MOVE;
        test_entry.node_type = PVNODE;
    }

    void TearDown( ) { 

    }

    ~TTableF( )  { 

    }

};

TEST_F(TTableF, GetBytes) {
    EXPECT_NEAR(TEST_T_SIZE, t.get_bytes(), sizeof(tt_entry));
}

TEST_F(TTableF, Resize) {
    U64 n = t.resize(10E6);
    EXPECT_NEAR(n, t.get_bytes()/sizeof(tt_entry), 1);
    EXPECT_NEAR(10E6, t.get_bytes(), sizeof(tt_entry));
    t.resize(32E6);
    EXPECT_NEAR(32E6, t.get_bytes(), sizeof(tt_entry));
    t.resize(16E6);
    EXPECT_NEAR(16E6, t.get_bytes(), sizeof(tt_entry));
    EXPECT_EQ(t.resize(t.get_bytes()), t.resize(t.get_bytes()));
}

TEST_F(TTableF, MinTableSize) {
    t.resize(0);
    EXPECT_GT(t.get_bytes(),0); //there should be a min table size
    t.resize(1);
    EXPECT_GT(t.get_bytes(),0); //there should be a min table size
}

TEST_F(TTableF, Hashfull) {
    EXPECT_EQ(t.hashfull(), 0);
    for(auto i=0; i<t.size();i++){
        test_entry.full_key = i;
        t.place(test_entry);
    }
    EXPECT_EQ(t.hashfull(), 1E6); //1 million should always be the max value for UCI hashfull
    t.reset();
    EXPECT_EQ(t.hashfull(), 0);
}

TEST_F(TTableF, PlaceAndFind) {
    //this test could be better
    //maybe include a test for replacement scheme??
    int alpha = -1;
    int beta = 1;
    int score = 0;
    test_entry.full_key = 100;
    t.place(test_entry);
    EXPECT_EQ(t.find(100,&score,&alpha,&beta,1,1) , BEST_MOVE);
    EXPECT_EQ(score, 25);
    EXPECT_EQ(alpha, -1);
    EXPECT_EQ(beta, 1);
}

TEST_F(TTableF, AlphaBetaFind) {
    int alpha = -1;
    int beta = 1;
    int score = 0;
    for(auto i=0; i<t.size();i++){
        test_entry.full_key = i;

        //randomly make node affect either beta or alpha
        test_entry.node_type = (rand()%2 == 1) ? CUTNODE : ALLNODE;

        //give a random score;
        test_entry.score = rand()%2001-1000;

        t.place(test_entry);
        t.find(i,&score,&alpha,&beta,1,1);
    }
    EXPECT_NEAR(alpha, 1000, 10);
    EXPECT_NEAR(beta, -1000, 10);
    EXPECT_EQ(score, 0);
}








