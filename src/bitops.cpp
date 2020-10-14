
#include "bitops.h"
#include <bit>

unsigned char bitops::bscanf64(unsigned long* index, U64 mask)
{
    for(int i=0;i<64;i++){
        if (((U64)1 << i) & mask){
            *index = i;
            return 1;
        } 
    }
    return 0;
}

unsigned char bitops::bscanr64(unsigned long* index, U64 mask)
{
    for(int i=63;i>=0;i--){
        if (((U64)1 << i) & mask){
            *index = i;
            return 1;
        } 
    }
    return 0;
}

unsigned int bitops::popcount64(U64 mask)
{
    unsigned int n = 0;
    for(int i=0;i<64;i++){
        if (((U64)1 << i) & mask) n++;
    }
    return n;
}
