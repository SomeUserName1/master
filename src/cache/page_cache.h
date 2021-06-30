#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stddef.h>

#include "data-struct/bitmap.h"
#include "data-struct/linked_list.h"
#include "page.h"
#include "physical_database.h"

typedef struct
{
    phy_database* pdb;
    size_t        total_pinned;
    size_t        total_unpinned;
    bitmap*       pinned;
    queue_ul*     recently_referenced;
    dict_ul_page* page_map; /* Page M is stored in frame N */
    page*         cache[];  /* Frame N contains page M */
} page_cache;

page_cache*
page_cache_create(phy_database* pdb);

void
page_cache_destroy(page_cache* pc);

page*
pin_page(page_cache* pc, size_t page_no, record_file rf);

void
unpin_page(page_cache* pc, size_t page_no);

size_t
evict_page(page_cache* pc);

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

void
rp_passtrough_fn(page_cache* pc);

void
rp_passthrough_pin(page_cache* pc, size_t page_no);

void
rp_queue_init(page_cache* pc);

void
rp_queue_clean(page_cache* pc);

size_t
random_evict(page_cache* pc);

void
random_handle_pin(page_cache* pc, size_t page_no);

void
random_handle_unpin(page_cache* pc, size_t page_no);

size_t
fifo_evict(page_cache* pc);

void
fifo_handle_pin(page_cache* pc, size_t page_no);

void
fifo_handle_unpin(page_cache* pc, size_t page_no);

size_t
lru_evict(page_cache* pc);

size_t
lru_k_evict(page_cache* pc);

void
lru_handle_pin(page_cache* pc, size_t page_no);

void
lru_handle_unpin(page_cache* pc, size_t page_no);

void
clock_init(page_cache* pc);

size_t
clock_evict(page_cache* pc);

void
clock_handle_pin(page_cache* pc, size_t page_no);

void
clock_handle_unpin(page_cache* pc, size_t page_no);

size_t
gclock_evict(page_cache* pc);

void
gclock_handle_pin(page_cache* pc, size_t page_no);

void
gclock_handle_unpin(page_cache* pc, size_t page_no);

#endif
