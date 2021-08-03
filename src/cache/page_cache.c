#include "page_cache.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/bitmap.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "disk_file.h"
#include "page.h"
#include "physical_database.h"

#define SWAP_MAX_NUM_PINNED_PAGES (6)

page_cache*
page_cache_create(phy_database* pdb, const char* log_path)
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
    pc->free_frames         = ll_ul_create();
    pc->pinned              = bitmap_create(CACHE_N_PAGES);
    pc->recently_referenced = q_ul_create();

    for (unsigned long i = 0; i < invalid; ++i) {
        pc->page_map[i] = d_ul_ul_create();
    }

    unsigned char* data = calloc(CACHE_N_PAGES, PAGE_SIZE);

    if (!data) {
        printf("page cache - create: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    for (unsigned long i = 0; i < CACHE_N_PAGES; ++i) {
        pc->cache[i] = page_create(ULONG_MAX, data + (PAGE_SIZE * i));
        llist_ul_append(pc->free_frames, i);
    }

    FILE* log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("heap file - create: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }
    pc->log_file = log_file;

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
    llist_ul_destroy(pc->free_frames);
    queue_ul_destroy(pc->recently_referenced);

    for (size_t i = 0; i < invalid; ++i) {
        dict_ul_ul_destroy(pc->page_map[i]);
    }

    for (unsigned long i = 0; i < CACHE_N_PAGES; ++i) {
        page_destroy(pc->cache[i]);
    }

    fclose(pc->log_file);

    free(pc);
}

unsigned long
new_page(page_cache* pc, file_type ft)
{
    if (!ft || (ft != node_file && ft != relationship_file)) {
        printf("page cache - pin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    allocate_pages(pc->pdb, ft, 1);
    return pc->pdb->files[ft]->num_pages - 1;
}

page*
pin_page(page_cache* pc, size_t page_no, file_type ft)
{
    if (!pc || page_no > MAX_PAGE_NO) {
        printf("page cache - pin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t frame_no;
    page*  pinned_page;
    if (dict_ul_ul_contains(pc->page_map[ft], page_no)) {
        frame_no    = dict_ul_ul_get_direct(pc->page_map[ft], page_no);
        pinned_page = pc->cache[frame_no];
        pinned_page->pin_count++;
    } else {
        if (llist_ul_size(pc->free_frames) > 0) {
            frame_no = llist_ul_take(pc->free_frames, 0);
        } else {
            frame_no = evict_page(pc);
        }

        pinned_page = pc->cache[frame_no];

        pinned_page->ft        = ft;
        pinned_page->page_no   = page_no;
        pinned_page->pin_count = 1;
        pinned_page->dirty     = false;

        disk_file* df = pc->pdb->files[ft];

        read_page(df, page_no, pinned_page->data);

        dict_ul_ul_insert(pc->page_map[ft], page_no, frame_no);
    }

    set_bit(pc->pinned, frame_no);

    pc->total_pinned++;

#ifdef VERBOSE
    fprintf(pc->log_file,
            "Pin %s %lu\n",
#ifdef ADJLIST
            ft == record_file = "record"
            : "header",
#else
            FILE_STRING[ft],
#endif
              page_no);
#endif
    return pinned_page;
}

void
unpin_page(page_cache* pc, size_t page_no, file_type ft)
{
    if (!pc || page_no > MAX_PAGE_NO
        || !dict_ul_ul_contains(pc->page_map[ft], page_no)) {
        printf("page cache - unpin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t frame_no      = dict_ul_ul_get_direct(pc->page_map[ft], page_no);
    page*  unpinned_page = pc->cache[frame_no];

    if (unpinned_page->pin_count == 0) {
        printf("page cache - unpin page: Page %zu is not pinned (page count is "
               "zero)!\n",
               page_no);
        exit(EXIT_FAILURE);
    }

    unpinned_page->pin_count--;

    if (unpinned_page->pin_count == 0) {
        clear_bit(pc->pinned, frame_no);
    }

    if (queue_ul_contains(pc->recently_referenced, frame_no)) {
        queue_ul_remove_elem(pc->recently_referenced, frame_no);
    }

    queue_ul_push(pc->recently_referenced, frame_no);

    pc->total_unpinned++;

#ifdef VERBOSE
    fprintf(pc->log_file, "Unpin %s %lu\n", FILE_STRING[ft], page_no);
#endif
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
    for (size_t i = 0; i < queue_ul_size(pc->recently_referenced); ++i) {
        evict = queue_ul_get(pc->recently_referenced, i);
        if (get_bit(pc->pinned, evict) == 0) {

            /* If the page is dirty flush it */
            if (pc->cache[evict]->dirty) {
                flush_page(pc, evict);
            }
#ifdef VERBOSE
            fprintf(pc->log_file, "evict %lu\n", pc->cache[evict]->page_no);
#endif
            /* Remove freed frame from the recently referenced queue */
            queue_ul_remove(pc->recently_referenced, i);
            /* Remove reference of page from lookup table */
            dict_ul_ul_remove(pc->page_map[pc->cache[evict]->ft],
                              pc->cache[evict]->page_no);
            /* Add the frame to the free frames list */
            llist_ul_append(pc->free_frames, i);

            pc->cache[evict]->page_no = UNINITIALIZED_LONG;
            pc->cache[evict]->ft      = invalid;

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
flush_page(page_cache* pc, size_t frame_no)
{
    if (!pc) {
        printf("page cache - flush page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (pc->cache[frame_no]->dirty) {

        disk_file* df = pc->pdb->files[pc->cache[frame_no]->ft];

        write_page(df, pc->cache[frame_no]->page_no, pc->cache[frame_no]->data);
        df->write_count++;

        pc->cache[frame_no]->dirty = false;
    }

#ifdef VERBSE
    fprintf(pc->log_file, "Flushed %lu\n", pc->cache[frame_no]->page_no);
#endif
}

void
flush_all_pages(page_cache* pc)
{
    for (unsigned long i = 0; i < CACHE_N_PAGES; ++i) {
        if (pc->cache[i]->dirty) {
            flush_page(pc, pc->cache[i]->page_no);
        }
    }
}
