#ifndef CACHE_H
#define CACHE_H

#include "../data-types/dict_ul.h"

typedef enum replacement_policy
{
    FIFO = 0,
} replacement_policy_t;

typedef struct cache
{
    frame_t page_cache[];
    dict_ul_frame_t* cache_map;
    replacement_policy_t policy;
    list_frame_t* free_pages;
} cache_t;

#endif
