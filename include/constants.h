#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <float.h>
#include <stddef.h>

static const unsigned long UNINITIALIZED_LONG    = 0xFFFFFFFFFFFFF666;
static const unsigned char UNINITIALIZED_BYTE    = 0x00;
static const double        UNINITIALIZED_WEIGHT  = 0x1.FFFFFFFFFF666p-1;
static const unsigned char FIRST_REL_SOURCE_FLAG = 0x02;
static const unsigned char FIRST_REL_TARGET_FLAG = 0x04;

#define PAGE_SIZE     (4096)
#define CACHE_SIZE    (2 << 30) /* 2 GiB */
#define CACHE_N_PAGES (CACHE_SIZE / PAGE_SIZE)
#define MAX_PAGE_NO   (LONG_MAX / PAGE_SIZE)

#define MAX_STR_LEN (32)

#define EVICT_LRU_K (42)

#endif
