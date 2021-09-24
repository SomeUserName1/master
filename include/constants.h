/*
 * @(#)constants.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <float.h>
#include <stddef.h>

static const unsigned long UNINITIALIZED_LONG   = 0xFFFFFFFFFFFFF666;
static const unsigned char UNINITIALIZED_BYTE   = 0xFF;
static const double        UNINITIALIZED_WEIGHT = 0x1.FFFFFFFFFF666p-1;

/* The size of a page, the maximum number of pages (limited by the standard
 * library using the type long for file offsets) and the maximum amount of
 * characters in a string to be stored to disk. */
#define PAGE_SIZE                                                              \
    (4UL << 10) /* 4 KiB TODO use unistd.h to fetch od page size */
#define MAX_PAGE_NO                                                            \
    (LONG_MAX / PAGE_SIZE) /* Approx 1 << 32 - 1 pages; Overall Maximum        \
                              size of the database in bytes is 8 EiB */
#define MAX_STR_LEN (31)

#define SLOT_SIZE      (16)
#define SLOTS_PER_PAGE (4096 / 16)

/* size of the cache for the actual graph */
#define CACHE_SIZE    (PAGE_SIZE * 10000)
#define CACHE_N_PAGES (CACHE_SIZE / PAGE_SIZE)

/* The implemented LRU-K evicts 1 + CACHE_N_PAGES * EVICT_LRU_SHARE pages
 * per call to evict. For example with a cache size of 1k pages, 101 pages
 * would be evicted per call */
static const float EVICT_LRU_K_SHARE = 0.1F;
#define EVICT_LRU_K ((1 + (size_t)((size_t)CACHE_N_PAGES * EVICT_LRU_K_SHARE)))

#endif
