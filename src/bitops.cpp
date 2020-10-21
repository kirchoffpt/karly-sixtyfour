
#include "bitops.h"
#include <bit>
#ifdef _WIN32
#include "immintrin.h"
#include <intrin.h>
#endif

#include<iostream>

unsigned char bitops::bscanf64(unsigned long* index, U64 mask)
{
    #ifdef __linux__
        unsigned long idx = __builtin_ffsll(mask);
        if(idx){
            *index = idx-1;
            return 1;
        } else {
            return 0;
        }
       
    #elif _WIN32
        return _BitScanForward64(index, mask);
    #else
        for(int i=0;i<64;i++){
            if (((U64)1 << i) & mask){
                *index = i;
                return 1;
            } 
        }
        return 0;
    #endif
}

unsigned char bitops::bscanr64(unsigned long* index, U64 mask)
{
    #ifdef __linux__
        if(mask == 0) return 0;
        unsigned long lz = __builtin_clzll(mask);
        *index = 63-lz;
        return 1;
    #elif _WIN32
        return _BitScanReverse64(index, mask);
    #else
        for(int i=63;i>=0;i--){
            if (((U64)1 << i) & mask){
                *index = i;
                return 1;
            } 
        }
        return 0;
    #endif
}

unsigned int bitops::popcount64(U64 mask)
{
    #ifdef __linux__
    #ifdef __cpp_lib_bitops
        //CXX20
        return std::popcount(mask);
    #else
        return __builtin_popcountll(mask);
    #endif
    #elif _WIN32
        //MSVC does not support CXX20 popcount at this moment
        return __popcnt64(mask);
    #else
        //this is backup, very slow
        unsigned int n = 0;
        for(int i=0;i<64;i++){
            if (((U64)1 << i) & mask) n++;
        }
        return n;
    #endif
}
