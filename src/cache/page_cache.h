/*
 * @(#)page_cache.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stddef.h>

#include "constants.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "page.h"
#include "physical_database.h"

typedef struct
{
    phy_database* pdb;
    size_t        num_pins;
    size_t        num_unpins;
    bool          bulk_import;
#ifdef VERBOSE
    FILE* log_file;
#endif
    llist_ul*   free_frames;
    queue_ul*   recently_referenced;
    dict_ul_ul* page_map[invalid]
                        [invalid_ft]; /* Page M is stored in frame N. Invalid =
                                         num file kinds, invalid_ft = num file
                                         types. See physical_database.h*/
    page* cache[CACHE_N_PAGES];       /* Frame N contains page M */
} page_cache;

page_cache*
page_cache_create(phy_database* pdb
#ifdef VERBOSE
                  ,
                  const char* log_path
#endif
);

void
page_cache_destroy(page_cache* pc);

page*
pin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft);

void
unpin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft);

size_t
evict_page(page_cache* pc);

size_t
bulk_evict(page_cache* pc);

void
flush_page(page_cache* pc, size_t frame_no);

void
flush_all_pages(page_cache* pc);

page*
new_page(page_cache* pc, file_type ft);

#endif
