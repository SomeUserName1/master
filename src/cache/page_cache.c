#include "page_cache.h"

#include <limits.h>
#include <stdlib.h>

#include "constants.h"
#include "data-struct/bitmap.h"
#include "data-struct/linked_list.h"
#include "disk_file.h"
#include "page.h"
#include "physical_database.h"

page_cache*
page_cache_create(phy_database* pdb)
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

    pc->pdb                 = pdb;
    pc->total_pinned        = 0;
    pc->total_unpinned      = 0;
    pc->pinned              = bitmap_create(CACHE_N_PAGES);
    pc->recently_referenced = q_ul_create();
    pc->page_map            = d_ul_page_create();

    unsigned char* data = calloc(CACHE_N_PAGES, PAGE_SIZE);

    if (!data) {
        printf("page cache - create: failed to allocate memory!\n");
    }

    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        pc->cache[i] = page_create(ULONG_MAX, data + (PAGE_SIZE * i));
    }

    return pc;
}

void
page_cache_destroy(page_cache* pc)
{
    if (!pc) {
        printf("page_cache - destroy: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    free(pc->pinned);
    queue_ul_destroy(pc->recently_referenced);
    dict_ul_page_destroy(pc->page_map);

    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        page_destroy(pc->cache[i]);
    }

    free(pc);
}

page*
pin_page(page_cache* pc, size_t page_no, record_file rf)
{
    if (!pc || page_no > MAX_PAGE_NO) {
        printf("page cache - pin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    page* pinned_page;
    if (dict_ul_page_contains(pc->page_map, page_no)) {
        pinned_page = dict_ul_page_get_direct(pc->page_map, page_no);
        pinned_page->pin_count++;
    } else {
        if (all_bits_set(pc->pinned)) {
            pinned_page = pc->cache[evict_page(pc)];
        } else {
            pinned_page = pc->cache[get_first_unset(pc->pinned)];
        }

        pinned_page->page_no   = page_no;
        pinned_page->dirty     = false;
        pinned_page->pin_count = 1;
        pinned_page->rf        = rf;

        disk_file* df;

        // FIXME records or headers... how?
        if (rf == node_file) {
            df = pc->pdb->node_file;
        } else {
            df = pc->pdb->rel_file;
        }

        read_page(df, page_no, pinned_page->data);
    }

    set_bit(pc->pinned, page_no);

    pc->total_pinned++;

    return pinned_page;
}

void
unpin_page(page_cache* pc, size_t page_no)
{
    if (!pc || page_no > MAX_PAGE_NO
        || !dict_ul_page_contains(pc->page_map, page_no)) {
        printf("page cache - unpin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    page* unpinned_page = dict_ul_page_get_direct(pc->page_map, page_no);

    if (unpinned_page->pin_count == 0) {
        printf("page cache - unpin page: Page %zu is not pinned (page count is "
               "zero)!\n",
               page_no);
        exit(EXIT_FAILURE);
    }

    unpinned_page->pin_count--;

    if (unpinned_page->pin_count == 0) {
        clear_bit(pc->pinned, page_no);
    }

    if (queue_ul_contains(pc->recently_referenced, page_no)) {
        queue_ul_remove_elem(pc->recently_referenced, page_no);
    }

    queue_ul_push(pc->recently_referenced, page_no);

    pc->total_unpinned++;
}

size_t
evict_page(page_cache* pc)
{
    if (!pc) {
        printf("page cache - evict page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t evict;
    size_t evicted = 0;
    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        evict = queue_ul_get(pc->recently_referenced, i);
        if (get_bit(pc->pinned, evict) == 0) {
            if (pc->cache[evict]->dirty) {
                flush_page(pc, evict);
            }

            queue_ul_remove(pc->recently_referenced, i);
            evicted++;

            if (evicted >= EVICT_LRU_K) {
                break;
            }
        }
    }
    if (evicted == 0) {
        printf("page cache - evict: could not find a page to evict, as all "
               "pages "
               "are pinned!\n");
        exit(EXIT_FAILURE);
    }
    return evict;
}

void
flush_page(page_cache* pc, size_t page_no)
{
    if (!pc) {
        printf("page cache - flush page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    // FIXME Records or headers how?
    disk_file* df;
    if (pc->cache[page_no]->rf == node_file) {
        df = pc->pdb->node_file;
    } else if (pc->cache[page_no]->rf == relationship_file) {
        df = pc->pdb->rel_file;
    }

    write_page(df, page_no, pc->cache[page_no]->data);
    df->write_count++;
}

void
flush_all_pages(page_cache* pc)
{
    for (int i = 0; i < CACHE_N_PAGES; ++i) {
        if (pc->cache[i]->dirty) {
            flush_page(pc, pc->cache[i]->page_no);
        }
    }
}

