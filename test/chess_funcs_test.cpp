#include <limits.h>
#include "chess_funcs.h"
#include "gtest/gtest.h"

TEST(ChessFuncs, IdxToCoord) {
  EXPECT_EQ(idx_to_coord(0), "h1");
  EXPECT_EQ(idx_to_coord(1), "g1");
  EXPECT_EQ(idx_to_coord(63), "a8");
}


