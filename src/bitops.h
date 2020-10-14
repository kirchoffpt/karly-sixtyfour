#ifndef BITOPS_H
#define BITOPS_H

#define U64 unsigned long long int

namespace bitops {

unsigned char bscanf64(unsigned long* index, U64 mask);
unsigned char bscanr64(unsigned long* index, U64 mask);
unsigned int popcount64(U64 n);

}

#endif

