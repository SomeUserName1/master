#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stddef.h>

#include "data-struct/linked_list.h"
#include "page.h"
#include "physical_database.h"

typedef enum
{
    RANDOM,
    FIFO,
    LRU,
    LRU_K,
    CLOCK,
    GCLOCK
} replacement_policy;

typedef struct pg_cch
{
    phy_database*     pdb;
    size_t            total_pinned;
    size_t            total_unpinned;
    linked_list_page* free_pages;
    /*
     * Data structures and function pointers used for evicitions
     * We use a queue here as FIFO
     * and LRU are using a queue and the queue
     * can be used as list, s.t. CLOCK and GCLOCK can
     * also be implemented.
     */
    replacement_policy policy;
    void (*rp_init)(struct pg_cch*);
    void (*rp_clean)(struct pg_cch*);
    void (*rp_handle_pin)(struct pg_cch*, size_t page_no);
    void (*rp_handle_unpin)(struct pg_cch*, size_t page_no);
    void (*rp_evict)(struct pg_cch*);
    size_t    evict_current;
    queue_ul* evict_references;

    dict_ul_page* page_map; /* Page M is stored in frame N */
    page*         cache[];  /* Frame N contains page M */
} page_cache;

page_cache*
page_cache_create(phy_database* pdb, replacement_policy rp);

void
page_cache_destroy(page_cache* pc);

page*
pin_page(page_cache* pc, size_t page_no);

void
unpin_page(page_cache* pc, size_t page_no);

void
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
random_evict(page_cache* pc);

void
random_handle_pin(page_cache* pc, size_t page_no);

void
random_handle_unpin(page_cache* pc, size_t page_no);

void
rp_queue_init(page_cache* pc);

void
rp_queue_clean(page_cache* pc);

void
fifo_evict(page_cache* pc);

void
fifo_handle_pin(page_cache* pc, size_t page_no);

void
fifo_handle_unpin(page_cache* pc, size_t page_no);

void
lru_evict(page_cache* pc);

void
lru_k_evict(page_cache* pc);

void
lru_handle_pin(page_cache* pc, size_t page_no);

void
lru_handle_unpin(page_cache* pc, size_t page_no);

void
clock_init(page_cache* pc);

void
clock_evict(page_cache* pc);

void
clock_handle_pin(page_cache* pc, size_t page_no);

void
clock_handle_unpin(page_cache* pc, size_t page_no);
void
gclock_evict(page_cache* pc);

void
gclock_handle_pin(page_cache* pc, size_t page_no);

void
gclock_handle_unpin(page_cache* pc, size_t page_no);

#endif
