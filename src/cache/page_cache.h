#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stddef.h>

#include "page.h"
#include "physical_database.h"

typedef enum
{
    RANDOM = 0,
    FIFO   = 1,
    LRU    = 2,
    CLOCK  = 3
} replacement_policy;

typedef struct
{
    phy_database*      pdb;
    replacement_policy policy;
    linked_list_page*  free_pages;
    dict_ul_page*      page_map;
    size_t             total_pinned;
    size_t             total_unpinned;
    page               cache[];
} page_cache;

page_cache*
page_cache_create(phy_database* pdb, replacement_policy rp);

void
page_cache_destroy(page_cache* pdb);

page*
pin_page(page_cache* pc, size_t page_no);

void
unpin_page(page_cache* pc, size_t page_no);

page*
copy_page(page_cache* pc, size_t to_copy_page_no);

page*
new_empty_page(page_cache* pc);

void
delete_page(page_cache* pc, size_t page_no);

void
flush_page(page_cache* pc, size_t page_no);

void
flush_all_pages(page_cache* pc, size_t page_no);

#endif
