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

TEST(BitOps, BitScanForward64) {
  unsigned long idx;
  U64 mask = 0x8000000000000000;
  for(int i=0;i<64;i++){
   bscanf64(&idx, mask | (mask >> i));
   ASSERT_EQ(idx,63-i);
  }
}

TEST(BitOps, BitScanReverse64) {
  unsigned long idx;
  U64 mask = 0x1;
  for(int i=0;i<64;i++){
   bscanr64(&idx, mask | (mask << i));
   ASSERT_EQ(idx,i);
  }
}

TEST(BitOps, BitScanForward64Zero) {
  unsigned long idx;
  U64 mask = 0x0;
  EXPECT_FALSE(bscanf64(&idx, mask));
}

TEST(BitOps, BitScanReverse64Zero) {
  unsigned long idx;
  U64 mask = 0x0;
  EXPECT_FALSE(bscanr64(&idx, mask));
}

}





