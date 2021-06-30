#include "page_cache.h"

#include <limits.h>
#include <stdlib.h>

#include "constants.h"
#include "data-struct/linked_list.h"
#include "page.h"
#include "physical_database.h"

page_cache*
page_cache_create(phy_database* pdb, replacement_policy rp)
{
    if (!pdb) {
        printf("page cache - create: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    page_cache* pc = calloc(1, sizeof(page_cache));

    if (!pc) {
        printf("page cache - create: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    pc->pdb            = pdb;
    pc->total_pinned   = 0;
    pc->total_unpinned = 0;
    pc->free_pages     = ll_page_create();
    pc->policy         = rp;

    switch (pc->policy) {
        case RANDOM: {
            pc->rp_init         = rp_passtrough_fn;
            pc->rp_clean        = rp_passtrough_fn;
            pc->rp_handle_pin   = random_handle_pin;
            pc->rp_handle_unpin = random_handle_unpin;
            pc->rp_evict        = random_evict;
            break;
        }
        case FIFO: {
            pc->rp_init         = rp_queue_init;
            pc->rp_clean        = rp_queue_clean;
            pc->rp_handle_pin   = fifo_handle_pin;
            pc->rp_handle_unpin = fifo_handle_unpin;
            pc->rp_evict        = fifo_evict;
            break;
        }
        case LRU: {
            pc->rp_init         = rp_queue_init;
            pc->rp_clean        = rp_queue_clean;
            pc->rp_handle_pin   = lru_handle_pin;
            pc->rp_handle_unpin = lru_handle_unpin;
            pc->rp_evict        = lru_evict;
            break;
        }
        case LRU_K: {
            pc->rp_init         = rp_queue_init;
            pc->rp_clean        = rp_queue_clean;
            pc->rp_handle_pin   = lru_handle_pin;
            pc->rp_handle_unpin = lru_handle_unpin;
            pc->rp_evict        = lru_k_evict;
            break;
        }
        case CLOCK: {
            pc->rp_init         = clock_init;
            pc->rp_clean        = rp_queue_clean;
            pc->rp_handle_pin   = clock_handle_pin;
            pc->rp_handle_unpin = clock_handle_unpin;
            pc->rp_evict        = clock_evict;
            break;
        }
        case GCLOCK: {
            pc->rp_init         = clock_init;
            pc->rp_clean        = rp_queue_clean;
            pc->rp_handle_pin   = gclock_handle_pin;
            pc->rp_handle_unpin = gclock_handle_unpin;
            pc->rp_evict        = gclock_evict;
            break;
        }
    }
    pc->page_map = d_ul_page_create();

    unsigned char* data = calloc(CACHE_N_PAGES, PAGE_SIZE);

    if (!data) {
        printf("page cache - create: failed to allocate memory!\n");
    }

    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        pc->cache[i] = page_create(ULONG_MAX, data + (PAGE_SIZE * i));
        linked_list_page_append(pc->free_pages, pc->cache[i]);
    }

    pc->rp_init(pc);

    return pc;
}

void
page_cache_destroy(page_cache* pc)
{
    if (!pc) {
        printf("page_cache - destroy: Invalid Arguments!\n");
        exit(-1);
    }

    pc->rp_clean(pc);
    linked_list_page_destroy(pc->free_pages);
    dict_ul_page_destroy(pc->page_map);

    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        page_destroy(pc->cache[i]);
    }

    free(pc);
}

page*
pin_page(page_cache* pc, size_t page_no)
{}

void
unpin_page(page_cache* pc, size_t page_no)
{}

void
evict_page(void)
{}

page*
copy_page(page_cache* pc, size_t to_copy_page_no)
{}

page*
new_empty_page(page_cache* pc)
{}

void
delete_page(page_cache* pc, size_t page_no)
{}

void
flush_page(page_cache* pc, size_t page_no)
{}

void
flush_all_pages(page_cache* pc, size_t page_no)
{}

void
rp_passtrough_fn(page_cache* pc)
{
    pc->evict_current = pc->evict_current;
}

void
random_evict(page_cache* pc)
{}

void
random_handle_pin(page_cache* pc, size_t page_no)
{}

void
random_handle_unpin(page_cache* pc, size_t page_no)
{}

void
rp_queue_init(page_cache* pc)
{
    pc->evict_references = q_ul_create();
}

void
rp_queue_clean(page_cache* pc)
{
    queue_ul_destroy(pc->evict_references);
}

void
fifo_evict(page_cache* pc)
{}

void
fifo_handle_pin(page_cache* pc, size_t page_no)
{}

void
fifo_handle_unpin(page_cache* pc, size_t page_no)
{}

void
lru_evict(page_cache* pc)
{}

void
lru_k_evict(page_cache* pc)
{}

void
lru_handle_pin(page_cache* pc, size_t page_no)
{}

void
lru_handle_unpin(page_cache* pc, size_t page_no)
{}

void
clock_init(page_cache* pc)
{
    pc->evict_current    = 0;
    pc->evict_references = q_ul_create();

    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        queue_ul_append(pc->evict_references, 0);
    }
}

void
clock_evict(page_cache* pc)
{}

void
clock_handle_pin(page_cache* pc, size_t page_no)
{}

void
clock_handle_unpin(page_cache* pc, size_t page_no)
{}

void
gclock_evict(page_cache* pc)
{}

void
gclock_handle_pin(page_cache* pc, size_t page_no)
{}

void
gclock_handle_unpin(page_cache* pc, size_t page_no)
{}

