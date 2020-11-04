#include <limits.h>
#include "chess_funcs.h"
#include "constants.h"
#include "chess_pos.h"
#include "gtest/gtest.h"

//mostly minimal testing for various functions
//we usually don't care what happens when something is used improperly
//most of these are used only at a low level and error checking will just slows things down
//functions reserved for debugging like print statements are not tested

TEST(ChessFuncs, IdxToCoord) {
  EXPECT_EQ(idx_to_coord(27), "e4");
  EXPECT_EQ(idx_to_coord(0), "h1");
  EXPECT_EQ(idx_to_coord(63), "a8");
  EXPECT_EQ(idx_to_coord(56), "h8");
  EXPECT_EQ(idx_to_coord(7), "a1");
}

TEST(ChessFuncs, BoardDist) {
  EXPECT_EQ(board_dist(0,63), 14);
  EXPECT_EQ(board_dist(7,0), 7);
  EXPECT_EQ(board_dist(8,0), 1);
  EXPECT_EQ(board_dist(7,56), 14);
}

TEST(ChessFuncs, BoardDistReversalSum) {
  int chksum=0;
  for(int i=0; i<64; i++){
    for(int j=0; j<64;j++){
      chksum += board_dist(i,j);
      chksum -= board_dist(j,i);
    }
  }
  EXPECT_EQ(chksum,0);
}

TEST(ChessFuncs, GetRayDir) {
  EXPECT_EQ(get_ray_dir(7,0),E_DIR);
  EXPECT_EQ(get_ray_dir(0,56),N_DIR);
  EXPECT_EQ(get_ray_dir(56,63),W_DIR);
  EXPECT_EQ(get_ray_dir(63,7),S_DIR);
  EXPECT_EQ(get_ray_dir(7,56),NE_DIR);
  EXPECT_EQ(get_ray_dir(63,0),SE_DIR);
  EXPECT_EQ(get_ray_dir(8,1),SW_DIR);
  EXPECT_EQ(get_ray_dir(0,9),NW_DIR);
}

TEST(ChessFuncs, GetRayDirReversalSum) {
  int chksum=0;
  for(int i=0; i<64; i++){
    for(int j=0; j<64;j++){
      chksum += get_ray_dir(i,j);
      chksum -= get_ray_dir(j,i);
    }
  }
  EXPECT_EQ(chksum,0);
}

TEST(ChessFuncs, IsSlidingPiece) {
  EXPECT_TRUE(is_sliding_piece(BISHOP));
  EXPECT_TRUE(is_sliding_piece(QUEEN));
  EXPECT_TRUE(is_sliding_piece(ROOK));
  EXPECT_FALSE(is_sliding_piece(KNIGHT));
  EXPECT_FALSE(is_sliding_piece(PAWN));
  EXPECT_FALSE(is_sliding_piece(KING));
  EXPECT_FALSE(is_sliding_piece(B_PAWN));
}

//bittoidx input is always *assumed* to be nonzero 64 bit with 1 set bit
TEST(ChessFuncs, BitToIdx) {
  EXPECT_EQ(bit_to_idx(0x0000000000000001),0);
  EXPECT_EQ(bit_to_idx(0x0000000000000080),7);
  EXPECT_EQ(bit_to_idx(0x0100000000000000),56);
  EXPECT_EQ(bit_to_idx(0x8000000000000000),63);
}

TEST(ChessFuncs, BitToIdxRollingTest) {
  for(int i=0;i<64;i++){
    EXPECT_EQ(bit_to_idx((U64)1<<i),i);
  }
}

TEST(ChessFuncs, FloodFillKing) {
  chess_pos pos; //create chess pos for lookup table to avoid generating extra lookup tables

  // U64 flood_fill_king(U--64 king_loc, U64 enemy_control, chess_mask_LUT* mlut, int depth); //gives all the squares a king could travel to given so many moves in a row
  EXPECT_EQ(flood_fill_king(0x1,0x0,&pos.MLUT,10),DARK_SQUARES | LIGHT_SQUARES);
  EXPECT_EQ(flood_fill_king(0x1,G_FILE,&pos.MLUT,10),H_FILE);
  EXPECT_EQ(flood_fill_king(0x80,LIGHT_SQUARES,&pos.MLUT,10),DARK_SQUARES);
  EXPECT_EQ(flood_fill_king(0x8000000000000000,DARK_SQUARES,&pos.MLUT,10),LIGHT_SQUARES);
}

TEST(ChessFuncs, FloodFillKingMaze) {
  chess_pos pos; //create chess pos for lookup table to avoid generating extra lookup tables
  U64 maze = 0x007F228AFA027E00;
  //  .......x
  //  .1111111
  //  ..1...1o
  //  1...1.1.
  //  11111.1.
  //  ......1.
  //  .111111.
  //  ........
  EXPECT_EQ(flood_fill_king(0x0100000000000000,maze,&pos.MLUT,30),~maze);
}

TEST(ChessFuncs, EncodeSrcDst) {
  EXPECT_EQ(encode_move_srcdst("h1h2"),8<<DST_SHIFT);
  EXPECT_EQ(encode_move_srcdst("a8a1"),encode_move_srcdst(63,7));
  EXPECT_EQ(encode_move_srcdst("e2e4"),(11<<SRC_SHIFT)+(27<<DST_SHIFT));
}





