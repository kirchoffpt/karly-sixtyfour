
#include "bitops.h"

#ifdef _WIN32
#include "immintrin.h"
#include <intrin.h>
#ifdef _WIN64
//Cxx20 header
//#include <bit>
#endif
#else
//Cxx20 header
//#include <bit>
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
    #ifdef _WIN64
        return _BitScanForward64(index, mask);
    #else
        uint32_t top = mask >> 32;
        uint32_t bot = mask & 0xFFFFFFFF;
        if(_BitScanForward(index, bot)){
            return 1; //just nonzero
        }else{
            auto val =  _BitScanForward(index, top);
            *index += 32;
            return val;
        }
    #endif //_WIN64
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
    #ifdef _WIN64
        return _BitScanReverse64(index, mask);
    #else
        uint32_t top = mask >> 32;
        uint32_t bot = mask & 0xFFFFFFFF;
        if(_BitScanReverse(index, top)){
            *index += 32;
            return 1;
        }else{
            return _BitScanReverse(index, bot);
        }
    #endif //_WIN64
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
    //#ifdef __cpp_lib_bitops
        //CXX20
        //return std::popcount(mask);
        //the above compiles to the below function on linux anyway
    //#else
        return __builtin_popcountll(mask);
    //#endif
    #elif _WIN32
    #ifdef _WIN64
        //MSVC does not support CXX20 popcount at this moment
        return __popcnt64(mask);
    #else
        uint32_t top = mask >> 32;
        uint32_t bot = mask & 0xFFFFFFFF;
        return __popcnt(bot) + __popcnt(top);
    #endif //_WIN64
    #else
        //this is backup, very slow
        unsigned int n = 0;
        for(int i=0;i<64;i++){
            if (((U64)1 << i) & mask) n++;
        }
        return n;
    #endif
}
