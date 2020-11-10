#include <memory>
#include <vector>
#include "node_move_list.h"
#include "chess_funcs.h"
#include "constants.h"
#include "gtest/gtest.h"

#define MOVE_1 0x0810
#define MOVE_2 0x1c00
#define MOVE_3 0x1ff0
#define MOVE_4 0x03f0

U64 concat_shorts(unsigned short a, unsigned short b, unsigned short c, unsigned short d){
    return ((U64)d << 48) + ((U64)c << 32) + ((U64)b << 16) + (U64)a;
}

class NodeMoveListFull: public ::testing::Test { 
protected: 

    std::unique_ptr<node_move_list> ml = std::unique_ptr<node_move_list>(new node_move_list());
    unsigned short top_move; //top of list

    NodeMoveListFull( ){ 
    } 

    void SetUp( ) { 
        for(int i=0;i<MAX_MOVES_IN_POS;i++){
            top_move = encode_move_srcdst(i%64,(i+1)%64);
            ml->push_move(top_move); //fill list with garbage moves
        }
    }

    void TearDown( ) { 

    }

    ~NodeMoveListFull( )  { 

    }

};

class NodeMoveListSmall: public ::testing::Test { 
protected: 

    std::unique_ptr<node_move_list> ml = std::unique_ptr<node_move_list>(new node_move_list());

    NodeMoveListSmall( ){ 
    } 

    void SetUp( ) { 
        ml->push_move(MOVE_1);
        ml->push_move(MOVE_2);
        ml->push_move(MOVE_3);
        ml->push_move(MOVE_4);
    }

    void TearDown( ) { 

    }

    ~NodeMoveListSmall( )  { 

    }

};

TEST_F(NodeMoveListFull, MoveCount) {
    EXPECT_EQ(ml->get_num_moves(),MAX_MOVES_IN_POS);
}

TEST_F(NodeMoveListFull, GetMove) {
    EXPECT_EQ(ml->get_move(0),encode_move_srcdst(0,1));
    EXPECT_EQ(ml->get_move(MAX_MOVES_IN_POS-1),encode_move_srcdst((MAX_MOVES_IN_POS-1)%64,(MAX_MOVES_IN_POS)%64));
}

TEST_F(NodeMoveListFull, ResetGetPush) {
    ml->reset_num_moves();
    EXPECT_EQ(ml->get_num_moves(),0);
    EXPECT_FALSE(ml->get_move(0));
    EXPECT_FALSE(ml->get_move(MAX_MOVES_IN_POS-1));
    ml->push_move(0);
    EXPECT_EQ(ml->get_num_moves(),1);
}

TEST_F(NodeMoveListFull, PopMove) {
    int n = ml->get_num_moves();
    unsigned short move = ml->pop_move();
    EXPECT_EQ(ml->get_num_moves(),n-1);
    EXPECT_EQ(move, top_move);
    while(move = ml->pop_move()){}
    EXPECT_EQ(ml->get_num_moves(),0);
}

TEST_F(NodeMoveListFull, PushPromoCount) {
    ml->reset_num_moves();
    ml->push_promo_move(0);
    EXPECT_EQ(ml->get_num_moves(),4);
}

TEST_F(NodeMoveListFull, ContainsRandom) {
    for(int i=0; i<100; i++){
        EXPECT_TRUE(ml->contains(ml->get_random_move()));
    }
}

//*important test since memcpy is used in a technically undefined way*
TEST_F(NodeMoveListFull, MoveToFrontRandomChksum) {
    unsigned long long chksum = 0;
    unsigned short chkxor = 0;

    for(int i=0; i<ml->get_num_moves(); i++){
        chksum += ml->get_move(i);
        chkxor ^= ml->get_move(i);
    }
    
    //perform many random moves to front
    //assert that no moves are lost or changed
    for(int i=0; i<1E6; i++){
        ml->move_to_front(ml->get_random_move());
    }

    for(int i=0; i<ml->get_num_moves(); i++){
        chksum -= ml->get_move(i);
        chkxor ^= ml->get_move(i);
    }

    EXPECT_EQ(chksum,0);
    EXPECT_EQ(chkxor,0);
}

TEST_F(NodeMoveListSmall, SwapToFront) {
    ml->swap_to_front(MOVE_1);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_2,MOVE_3,MOVE_1));
    ml->swap_to_front(MOVE_1);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_2,MOVE_3,MOVE_1));
    ml->swap_to_front(MOVE_2);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_1,MOVE_3,MOVE_2));
    ml->swap_to_front(MOVE_3);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_1,MOVE_2,MOVE_3));
    ml->swap_to_front(MOVE_4);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_3,MOVE_1,MOVE_2,MOVE_4));
    EXPECT_EQ(ml->pop_move(),MOVE_4);
}

TEST_F(NodeMoveListSmall, MoveToFront) {
    ml->move_to_front(MOVE_1);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_2,MOVE_3,MOVE_4,MOVE_1));
    ml->move_to_front(MOVE_1);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_2,MOVE_3,MOVE_4,MOVE_1));
    ml->move_to_front(MOVE_2);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_3,MOVE_4,MOVE_1,MOVE_2));
    ml->move_to_front(MOVE_3);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_1,MOVE_2,MOVE_3));
    ml->move_to_front(MOVE_4);
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_1,MOVE_2,MOVE_3,MOVE_4));
    EXPECT_EQ(ml->pop_move(),MOVE_4);
}

TEST_F(NodeMoveListSmall, SortByList) {
    std::vector<int> s_list = {0,1,2,3};
    ml->sort_moves_by_scores(s_list.data());
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_1,MOVE_2,MOVE_3,MOVE_4));
    s_list = {3,2,1,0};
    ml->sort_moves_by_scores(s_list.data());
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_3,MOVE_2,MOVE_1));
    s_list = {2,2,2,2};
    ml->sort_moves_by_scores(s_list.data());
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_3,MOVE_2,MOVE_1));
    s_list = {2,3,3,3};
    ml->sort_moves_by_scores(s_list.data());
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_4,MOVE_3,MOVE_2,MOVE_1));
    s_list = {4,4,4,1};
    ml->sort_moves_by_scores(s_list.data());
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_1,MOVE_4,MOVE_3,MOVE_2));
    s_list = {0,0,0,0};
    ml->sort_moves_by_scores(s_list.data());
    EXPECT_EQ(*(U64*)ml->data(), concat_shorts(MOVE_1,MOVE_4,MOVE_3,MOVE_2));
}

TEST_F(NodeMoveListSmall, PopTarget) {
    uint16_t move = ml->pop_targeted_move((U64)1<<63);
    EXPECT_EQ(move, MOVE_4);
    move = ml->pop_targeted_move((U64)1<<63);
    EXPECT_EQ(move, MOVE_3);
    move = ml->pop_targeted_move((U64)1<<63);
    EXPECT_EQ(move, 1);
    ml->reset_num_moves();
    move = ml->pop_targeted_move((U64)1<<63);
    EXPECT_EQ(move, 0);
}

TEST_F(NodeMoveListSmall, PopAnyTarget) {
    ml->pop_targeted_move(~(U64)0);
    ml->pop_targeted_move(~(U64)0);
    ml->pop_targeted_move(~(U64)0);
    ml->pop_targeted_move(~(U64)0);
    EXPECT_EQ(ml->get_num_moves(), 0);
}





