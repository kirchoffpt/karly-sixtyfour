#include <memory>
#include <vector>
#include "chess_pos.h"
#include "constants.h"
#include "gtest/gtest.h"

#define MAX_RANDOM_MOVES 100
//these tests are not exhaustive

class ChessPosFixture: public ::testing::Test { 
protected: 

    chess_pos rpos, rpos2, rpos3;

    ChessPosFixture( ){ 
    } 

    void SetUp( ) { 
        rpos = chess_pos(STARTPOS);
        rpos.generate_moves();
        rpos.next = &rpos2;
        rpos2.next = &rpos3;
    }

    void TearDown( ) { 

    }

    ~ChessPosFixture( )  { 

    }

};

TEST_F(ChessPosFixture, CopyPosEq) {

    chess_pos pos(STARTPOS), pos1, pos2;
    pos.copy_pos(rpos);
    EXPECT_TRUE(pos == rpos);

    pos1.copy_pos(pos);
    pos2.copy_pos(pos);
    EXPECT_TRUE(pos1 == pos2);

    pos = rpos;
    EXPECT_TRUE(pos == rpos);
}

TEST_F(ChessPosFixture, Subtraction) {
    chess_pos pos = rpos; 
    uint16_t move;
    rpos.next = &pos;
    while(move = rpos.pop_and_add()){
        EXPECT_EQ(pos-rpos,move);
    }
}

TEST_F(ChessPosFixture, RandomSubtraction) {
    chess_pos prev = rpos; 
    uint16_t move;
    int n = 0;

    for(int i=0; i<250; i++){
        rpos = prev = chess_pos(STARTPOS);
        n=0;
        while(move = rpos.generate_and_add_random()){
            ASSERT_EQ(rpos-prev,move);
            prev = rpos;
            if(n++ > MAX_RANDOM_MOVES) break;
        }
    }
}

TEST_F(ChessPosFixture, PopAddClearOcc) {
    rpos.pop_and_add();
    rpos.next->generate_moves();
    rpos.next->pop_and_add();
    rpos.next->next->generate_moves();
    rpos.next->next->pop_and_add();
    rpos.clear_next_occs();
    EXPECT_NE(rpos.occ[0],0);
    EXPECT_NE(rpos.occ[1],0);
    EXPECT_TRUE((rpos2.occ[0] == rpos2.occ[1]) && (rpos2.occ[1]== 0));
    EXPECT_TRUE((rpos3.occ[0] == rpos3.occ[1]) && (rpos2.occ[1]== 0));
}

TEST_F(ChessPosFixture, FenAndZobrist){
    int n = 0, i;
    uint64_t k=0;
    chess_pos pos;

    for(i=0; i<5E3; i++){
        rpos = chess_pos(STARTPOS);
        //add a random number of random moves
        n=0;
        while(rpos.generate_and_add_random()){
            if(n++ > rand()%(MAX_RANDOM_MOVES*2)) break;
        }
        k += n;
        pos.load_new_fen(rpos.get_fen());
        ASSERT_TRUE(pos == rpos);
        ASSERT_EQ(pos.zobrist_key, rpos.zobrist_key);
    }
}

TEST_F(ChessPosFixture, NullMove){
    int i, n, temp;
    rpos.fwd_null_move();
    rpos2.generate_moves();
    EXPECT_EQ(rpos2.get_num_moves(), 20);
    rpos2.add_null_move();
    EXPECT_TRUE(rpos==rpos2);
    EXPECT_EQ(rpos2-rpos, 0);
    rpos.add_null_move();
    rpos.add_null_move();
    rpos.add_null_move();
    rpos.generate_moves();
    EXPECT_EQ(rpos.get_num_moves(),20);

    for(i=0; i<5E3; i++){
        rpos = chess_pos(STARTPOS);
        //add a random number of random moves
        n=0;
        while(rpos.generate_and_add_random()){
            if(n++ > rand()%(MAX_RANDOM_MOVES*2)) break;
        }
        rpos.generate_moves();
        temp = rpos.get_num_moves();
        rpos.add_null_move();
        rpos.add_null_move();
        rpos.generate_moves();
        ASSERT_EQ(temp, rpos.get_num_moves());
    }
}

TEST_F(ChessPosFixture, FirstMoveEvalEq){
    //black should have same starting eval as white if they were to move first
    rpos2 = rpos;
    rpos2.add_null_move();
    EXPECT_EQ(rpos2.eval(), -rpos.eval());
}

TEST_F(ChessPosFixture, OrderMoves){
    //move ordering will change a lot but it should at least preserve the number of moves
    rpos.order_moves();
    rpos.order_moves_smart();
    EXPECT_EQ(rpos.get_num_moves(), 20);
}








