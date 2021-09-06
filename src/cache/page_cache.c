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
page_cache_create(phy_database* pdb
#ifdef VERBOSE
                  ,
                  const char* log_path
#endif
)
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
    pc->num_pins            = 0;
    pc->num_unpins          = 0;
    pc->free_frames         = ll_ul_create();
    pc->pinned              = bitmap_create(CACHE_N_PAGES);
    pc->recently_referenced = q_ul_create();

    for (file_kind i = 0; i < invalid; ++i) {
        if (i == catalogue) {
            pc->page_map[i][0] = d_ul_ul_create();
            continue;
        }

        for (file_type j = 0; j < invalid_ft; ++j) {
            pc->page_map[i][j] = d_ul_ul_create();
        }
    }

    unsigned char* data = calloc(CACHE_N_PAGES, PAGE_SIZE);

    if (!data) {
        printf("page cache - create: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    for (unsigned long i = 0; i < CACHE_N_PAGES; ++i) {
        pc->cache[i] = page_create(data + (PAGE_SIZE * i));
        llist_ul_append(pc->free_frames, i);
    }

#ifdef VERBOSE
    FILE* log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("heap file - create: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }
    pc->log_file = log_file;
#endif

    return pc;
}

void
page_cache_destroy(page_cache* pc)
{
    if (!pc) {
        printf("page_cache - destroy: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    flush_all_pages(pc);

    llist_ul_destroy(pc->free_frames);
    queue_ul_destroy(pc->recently_referenced);
    bitmap_destroy(pc->pinned);

    for (unsigned long i = 0; i < invalid; ++i) {
        if (i == catalogue) {
            dict_ul_ul_destroy(pc->page_map[i][0]);
            continue;
        }

        for (unsigned long j = 0; j < invalid_ft; ++j) {
            dict_ul_ul_destroy(pc->page_map[i][j]);
        }
    }

    free(pc->cache[0]->data);

    for (unsigned long i = 0; i < CACHE_N_PAGES; ++i) {
        page_destroy(pc->cache[i]);
    }

#ifdef VERBOSE
    if (fclose(pc->log_file) != 0) {
        printf("page cache - destroy: Error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    free(pc);
}

page*
pin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft)
{
    if (!pc || (fk == catalogue && ft != 0)) {
        printf("page cache - pin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t num_pages;
    switch (fk) {
        case catalogue: {
            num_pages = pc->pdb->catalogue->num_pages;
            break;
        };
        case header: {
            num_pages = pc->pdb->header[ft]->num_pages;
            break;
        };
        case records: {
            num_pages = pc->pdb->records[ft]->num_pages;
            break;
        };
        case invalid: {
            printf("page cache - pin page: Invalid kind of file!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (page_no >= MAX_PAGE_NO || page_no >= num_pages) {
        printf("page cache - pin page: Page Number out of bounds!\n");
        exit(EXIT_FAILURE);
    }

    size_t frame_no;
    page*  pinned_page;
    if (dict_ul_ul_contains(pc->page_map[fk][ft], page_no)) {
        frame_no    = dict_ul_ul_get_direct(pc->page_map[fk][ft], page_no);
        pinned_page = pc->cache[frame_no];
        pinned_page->pin_count++;
    } else {
        if (llist_ul_size(pc->free_frames) > 0) {
            frame_no = llist_ul_take(pc->free_frames, 0);
        } else {
            frame_no = evict_page(pc);
        }

        pinned_page = pc->cache[frame_no];

        pinned_page->fk        = fk;
        pinned_page->ft        = ft;
        pinned_page->page_no   = page_no;
        pinned_page->pin_count = 1;
        pinned_page->dirty     = false;

        disk_file* df;
        switch (fk) {
            case catalogue: {
                df = pc->pdb->catalogue;
                break;
            };
            case header: {
                df = pc->pdb->header[ft];
                break;
            };
            case records: {
                df = pc->pdb->records[ft];
                break;
            };
            case invalid: {
                printf("page cache - pin page: Invalid kind of file!\n");
                exit(EXIT_FAILURE);
            }
        }

        read_page(df, page_no, pinned_page->data);

        dict_ul_ul_insert(pc->page_map[fk][ft], page_no, frame_no);
    }

    set_bit(pc->pinned, frame_no);

    pc->num_pins++;

#ifdef VERBOSE
    fprintf(pc->log_file, "Pin %lu %lu %lu\n", fk, ft, page_no);
#endif

    return pinned_page;
}

void
unpin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft)
{
    if (!pc || !dict_ul_ul_contains(pc->page_map[fk][ft], page_no)) {
        printf("page cache - unpin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t num_pages;
    switch (fk) {
        case catalogue: {
            num_pages = pc->pdb->catalogue->num_pages;
            break;
        };
        case header: {
            num_pages = pc->pdb->header[ft]->num_pages;
            break;
        };
        case records: {
            num_pages = pc->pdb->records[ft]->num_pages;
            break;
        };
        case invalid: {
            printf("page cache - pin page: Invalid kind of file!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (page_no >= MAX_PAGE_NO || page_no >= num_pages) {
        printf("page cache - pin page: Page Number out of bounds!\n");
        exit(EXIT_FAILURE);
    }

    size_t frame_no      = dict_ul_ul_get_direct(pc->page_map[fk][ft], page_no);
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

    queue_ul_move_back(pc->recently_referenced, frame_no);

    pc->num_unpins++;

#ifdef VERBOSE
    fprintf(pc->log_file, "Unpin %lu %lu %lu\n", fk, ft, page_no);
#endif
}

size_t
evict_page(page_cache* pc)
{
    if (!pc) {
        printf("page cache - evict page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t evict_index[EVICT_LRU_K];
    size_t candidate;
    page*  candidate_page;
    size_t evicted = 0;
    for (size_t i = 0; i < queue_ul_size(pc->recently_referenced); ++i) {
        candidate      = queue_ul_get(pc->recently_referenced, i);
        candidate_page = pc->cache[candidate];

        if (get_bit(pc->pinned, candidate) == 0) {

            /* If the page is dirty flush it */
            if (candidate_page->dirty) {
                flush_page(pc, candidate);
            }
#ifdef VERBOSE
            fprintf(pc->log_file, "evict %lu\n", candidate_page->page_no);
#endif
            /* Remove reference of page from lookup table */
            dict_ul_ul_remove(
                  pc->page_map[candidate_page->fk][candidate_page->ft],
                  candidate_page->page_no);
            /* Add the frame to the free frames list */
            llist_ul_append(pc->free_frames, i);

            pc->cache[candidate]->page_no = ULONG_MAX;
            pc->cache[candidate]->fk      = invalid;
            pc->cache[candidate]->ft      = invalid_ft;

            evict_index[evicted] = i;
            evicted++;

            if (evicted >= EVICT_LRU_K) {
                break;
            }
        }
    }

    for (size_t i = 0; i < evicted; ++i) {
        /* Remove freed frame from the recently referenced queue */
        /* From back to front to avoid that the indexes change when removing an
         * element */
        queue_ul_remove(pc->recently_referenced, evict_index[evicted - 1 - i]);
    }

    if (evicted == 0) {
        printf("page cache - evict: could not find a page to evict, as all "
               "pages "
               "are pinned!\n");
        exit(EXIT_FAILURE);
    }

    return candidate;
}

void
flush_page(page_cache* pc, size_t frame_no)
{
    if (!pc) {
        printf("page cache - flush page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }
    page* candidate = pc->cache[frame_no];

    if (candidate->pin_count > 0) {
        printf("page cache - flush page: Page %lu of file %u is pinned!\n",
               candidate->page_no,
               candidate->ft);
        exit(EXIT_FAILURE);
    }

    if (candidate->dirty) {

        disk_file* df;
        switch (candidate->fk) {
            case catalogue: {
                df = pc->pdb->catalogue;
                break;
            };
            case header: {
                df = pc->pdb->header[candidate->ft];
                break;
            };
            case records: {
                df = pc->pdb->records[candidate->ft];
                break;
            };
            case invalid: {
                printf("page cache - pin page: Invalid kind of file!\n");
                exit(EXIT_FAILURE);
            }
        }

        write_page(df, candidate->page_no, candidate->data);

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
        flush_page(pc, i);
    }
}

page*
new_page(page_cache* pc, file_type ft)
{
    if (!pc) {
        printf("page cache - new page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    allocate_pages(pc->pdb, ft, 1);
    return pin_page(pc, pc->pdb->records[ft]->num_pages - 1, records, ft);
}

