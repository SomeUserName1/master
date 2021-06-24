#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <float.h>
#include <stddef.h>

static const unsigned long UNINITIALIZED_LONG    = 0xFFFFFFFFFFFFF666;
static const unsigned char UNINITIALIZED_BYTE    = 0x00;
static const double        UNINITIALIZED_WEIGHT  = 0x1.FFFFFFFFFF666p-1;
static const unsigned char FIRST_REL_SOURCE_FLAG = 0x02;
static const unsigned char FIRST_REL_TARGET_FLAG = 0x04;

static const size_t BLOCK_SIZE = 512;
static const size_t PAGE_SIZE  = 4096;
#endif
