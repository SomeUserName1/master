/*!
 * \file constants.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief TODO
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <float.h>
#include <stddef.h>
#include <unistd.h>

static const unsigned long UNINITIALIZED_LONG   = 0xFFFFFFFFFFFFF666;
static const unsigned char UNINITIALIZED_BYTE   = 0xFF;
static const double        UNINITIALIZED_WEIGHT = 0x1.FFFFFFFFFF666p-1;

/*! The size of a page. Fetched from the OS to match an OS page to a DB page in
 * size. */
#define PAGE_SIZE ((size_t)sysconf(_SC_PAGESIZE))
#define MAX_PAGE_NO                                                            \
    (LONG_MAX / PAGE_SIZE) /* Approx 1 << 32 - 1 pages; Overall Maximum        \
                              size of the database in bytes is 8 EiB */

#define SLOT_SIZE      (16)
#define SLOTS_PER_PAGE (PAGE_SIZE / SLOT_SIZE)

/* size of the cache for the actual graph */
#define CACHE_SIZE    (PAGE_SIZE * 100)
#define CACHE_N_PAGES (CACHE_SIZE / PAGE_SIZE)

/* The implemented LRU-K evicts 1 + CACHE_N_PAGES * EVICT_LRU_SHARE pages
 * per call to evict. For example with a cache size of 1k pages, 101 pages
 * would be evicted per call */
static const float EVICT_LRU_K_SHARE = 0.1F;
#define EVICT_LRU_K ((1 + (size_t)((size_t)CACHE_N_PAGES * EVICT_LRU_K_SHARE)))

#endif
