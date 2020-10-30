#include "bitops.h"
#include "constants.h"
#include "gtest/gtest.h"

namespace bitops {

TEST(BitOps, Popcount64) {
  EXPECT_EQ(popcount64(0x0000000000000000), 0);
  EXPECT_EQ(popcount64(0xFFFFFFFFFFFFFFFF), 64);
  EXPECT_EQ(popcount64(0xF0F0F0F0F0F0F0F0), 32);
  EXPECT_EQ(popcount64(0xFFFFFFFF00000000), 32);
  EXPECT_EQ(popcount64(0x00000000FFFFFFFF), 32);
  EXPECT_EQ(popcount64(0x1111111111111111), 16);
  EXPECT_EQ(popcount64(0xAAAAAAAAAAAAAAAA), 32);
  EXPECT_EQ(popcount64(0x0000000000000001), 1);
}

// TEST(BitOps, BitScanForward64) {
//   EXPECT_EQ(popcount64(0x1), 1);
// }

// TEST(BitOps, BitScanReverse64) {
//   EXPECT_EQ(popcount64(0x1), 2);
// }

}





