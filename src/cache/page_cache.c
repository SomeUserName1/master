/*!
 * \file page_cache.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref page_cache.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "page_cache.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "disk_file.h"
#include "page.h"
#include "physical_database.h"

page_cache*
page_cache_create(phy_database* pdb, size_t n_frames, const char* log_path)
{
    if (!pdb) {
        // LCOV_EXCL_START
        printf("page cache - create: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    page_cache* pc = calloc(1, sizeof(page_cache));

    if (!pc) {
        // LCOV_EXCL_START
        printf("page cache - create: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    pc->pdb                 = pdb;
    pc->n_frames            = n_frames;
    pc->num_pins            = 0;
    pc->num_unpins          = 0;
    pc->free_frames         = ll_ul_create();
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

    unsigned char* data = calloc(n_frames, PAGE_SIZE);

    if (!data) {
        // LCOV_EXCL_START
        printf("page cache - create: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    pc->frames = calloc(n_frames, sizeof(page*));
    for (unsigned long i = 0; i < n_frames; ++i) {
        pc->frames[i] = page_create(data + (PAGE_SIZE * i));
        llist_ul_append(pc->free_frames, i);
    }

    FILE* log_file = fopen(log_path, "a");

    if (!log_file) {
        // LCOV_EXCL_START
        printf("heap file - create: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    pc->log_file = log_file;

    return pc;
}

void
page_cache_destroy(page_cache* pc)
{
    if (!pc) {
        // LCOV_EXCL_START
        printf("page_cache - destroy: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    flush_all_pages(pc, false);

    llist_ul_destroy(pc->free_frames);
    queue_ul_destroy(pc->recently_referenced);

    for (unsigned long i = 0; i < invalid; ++i) {
        if (i == catalogue) {
            dict_ul_ul_destroy(pc->page_map[i][0]);
            continue;
        }

        for (unsigned long j = 0; j < invalid_ft; ++j) {
            dict_ul_ul_destroy(pc->page_map[i][j]);
        }
    }

    free(pc->frames[0]->data);

    for (unsigned long i = 0; i < pc->n_frames; ++i) {
        page_destroy(pc->frames[i]);
    }
    free(pc->frames);

    if (fclose(pc->log_file) != 0) {
        // LCOV_EXCL_START
        printf("page cache - destroy: Error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    free(pc);
}

page*
pin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft, bool log)
{
    if (!pc || (fk == catalogue && ft != 0)) {
        // LCOV_EXCL_START
        printf("page cache - pin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            printf("page cache - pin page: Invalid kind of file!\n");
            exit(EXIT_FAILURE);
            // LCOV_EXCL_STOP
        }
    }

    if (page_no >= MAX_PAGE_NO || page_no >= num_pages) {
        // LCOV_EXCL_START
        printf("page cache - pin page: Page Number out of bounds! page no: "
               "%lu, file kind %u, file type %u\n",
               page_no,
               fk,
               ft);
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t frame_no;
    page*  pinned_page;
    if (dict_ul_ul_contains(pc->page_map[fk][ft], page_no)) {
        frame_no    = dict_ul_ul_get_direct(pc->page_map[fk][ft], page_no);
        pinned_page = pc->frames[frame_no];
        pinned_page->pin_count++;
    } else {
        if (llist_ul_size(pc->free_frames) == 0) {
            if (pc->bulk_import) {
                bulk_evict(pc);
            } else {
                evict(pc, log);
            }
        }

        frame_no = llist_ul_take(pc->free_frames, 0);

        pinned_page = pc->frames[frame_no];

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
                // LCOV_EXCL_START
                printf("page cache - pin page: Invalid kind of file!\n");
                exit(EXIT_FAILURE);
                // LCOV_EXCL_STOP
            }
        }

        read_page(df, page_no, pinned_page->data, log);

        dict_ul_ul_insert(pc->page_map[fk][ft], page_no, frame_no);
    }

    pc->num_pins++;

    if (log) {
        fprintf(pc->log_file, "Pin %u %u %lu\n", fk, ft, page_no);
        fflush(pc->log_file);
    }

    return pinned_page;
}

void
unpin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft, bool log)
{
    if (!pc || !dict_ul_ul_contains(pc->page_map[fk][ft], page_no)) {
        // LCOV_EXCL_START
        printf("page cache - unpin page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            printf("page cache - pin page: Invalid kind of file!\n");
            exit(EXIT_FAILURE);
            // LCOV_EXCL_STOP
        }
    }

    if (page_no >= MAX_PAGE_NO || page_no >= num_pages) {
        // LCOV_EXCL_START
        printf("page cache - pin page: Page Number out of bounds!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t frame_no      = dict_ul_ul_get_direct(pc->page_map[fk][ft], page_no);
    page*  unpinned_page = pc->frames[frame_no];

    if (unpinned_page->pin_count == 0) {
        // LCOV_EXCL_START
        printf("page cache - unpin page: Page %zu is not pinned (page count is "
               "zero)! file kind %u, file type %u\n",
               page_no,
               fk,
               ft);
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unpinned_page->pin_count--;

    if (!pc->bulk_import) {
        queue_ul_move_back(pc->recently_referenced, frame_no);
    }

    pc->num_unpins++;

    if (log) {
        fprintf(pc->log_file, "Unpin %u %u %lu\n", fk, ft, page_no);
        fflush(pc->log_file);
    }
}

void
evict(page_cache* pc, bool log)
{
    if (!pc) {
        // LCOV_EXCL_START
        printf("page cache - evict page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t evict_index[EVICT_LRU_K];
    size_t candidate;
    page*  candidate_page;
    size_t evicted = 0;
    for (size_t i = 0; i < queue_ul_size(pc->recently_referenced); ++i) {
        candidate      = queue_ul_get(pc->recently_referenced, i);
        candidate_page = pc->frames[candidate];

        if (candidate_page->pin_count == 0) {

            flush_page(pc, candidate, log);
            if (log) {
                fprintf(pc->log_file,
                        "Evict %u %u %lu\n",
                        candidate_page->fk,
                        candidate_page->ft,
                        candidate_page->page_no);
                fflush(pc->log_file);
            }
            /* Remove reference of page from lookup table */
            dict_ul_ul_remove(
                  pc->page_map[candidate_page->fk][candidate_page->ft],
                  candidate_page->page_no);
            /* Add the frame to the free frames list */
            llist_ul_append(pc->free_frames, candidate);

            candidate_page->page_no = ULONG_MAX;
            candidate_page->fk      = invalid;
            candidate_page->ft      = invalid_ft;
            memset(candidate_page->data, 0, PAGE_SIZE);

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
        // LCOV_EXCL_START
        printf("page cache - evict: could not find a page to evict, as all "
               "pages "
               "are pinned!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
}

void
bulk_evict(page_cache* pc)
{
    size_t evicted = 0;
    page*  c_page;
    for (size_t i = 0; i < pc->n_frames; ++i) {
        c_page = pc->frames[i];
        if (c_page->pin_count == 0) {
            flush_page(pc, i, false);

            dict_ul_ul_remove(pc->page_map[c_page->fk][c_page->ft],
                              c_page->page_no);

            /* Add the frame to the free frames list */
            llist_ul_append(pc->free_frames, i);

            c_page->page_no = ULONG_MAX;
            c_page->fk      = invalid;
            c_page->ft      = invalid_ft;
            memset(c_page->data, 0, PAGE_SIZE);

            evicted++;
        }
    }

    if (evicted == 0) {
        // LCOV_EXCL_START
        printf(
              "page cache - bulk evict: could not find a page to evict, as all "
              "pages "
              "are pinned!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
}

void
flush_page(page_cache* pc, size_t frame_no, bool log)
{
    if (!pc) {
        // LCOV_EXCL_START
        printf("page cache - flush page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    page* candidate = pc->frames[frame_no];

    if (candidate->pin_count > 0) {
        // LCOV_EXCL_START
        printf("page cache - flush page: Page %lu of file %u is pinned!\n",
               candidate->page_no,
               candidate->ft);
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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
                // LCOV_EXCL_START
                printf("page cache - pin page: Invalid kind of file!\n");
                exit(EXIT_FAILURE);
                // LCOV_EXCL_STOP
            }
        }

        write_page(df, candidate->page_no, candidate->data, log);

        pc->frames[frame_no]->dirty = false;
    }

    if (log) {
        fprintf(pc->log_file,
                "Flushed %u %u %lu\n",
                pc->frames[frame_no]->fk,
                pc->frames[frame_no]->ft,
                pc->frames[frame_no]->page_no);
        fflush(pc->log_file);
    }
}

void
flush_all_pages(page_cache* pc, bool log)
{
    for (unsigned long i = 0; i < pc->n_frames; ++i) {
        flush_page(pc, i, log);
    }
}

page*
new_page(page_cache* pc, file_type ft, bool log)
{
    if (!pc) {
        // LCOV_EXCL_START
        printf("page cache - new page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    allocate_pages(pc->pdb, ft, 1, log);
    return pin_page(pc, pc->pdb->records[ft]->num_pages - 1, records, ft, log);
}

void
page_cache_swap_log_file(page_cache* pc, const char* log_file_path)
{

    if (!pc || !log_file_path) {
        // LCOV_EXCL_START
        printf("page cache - swap log file: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (fclose(pc->log_file) != 0) {
        // LCOV_EXCL_START
        printf("page_cache - swap log file: Error closing file: %s",
               strerror(errno));
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    pc->log_file = fopen(log_file_path, "a");

    if (!pc->log_file) {
        // LCOV_EXCL_START
        printf("page cache - swap log file: failed to fopen %s: %s\n",
               log_file_path,
               strerror(errno));
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
}

void
page_cache_change_n_frames(page_cache* pc, size_t n_frames)
{
    if (!pc) {
        // LCOV_EXCL_START
        printf("page cache - swap log file: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    flush_all_pages(pc, false);

    for (size_t i = 0; i < pc->n_frames; ++i) {
        if (pc->frames[i]->pin_count != 0) {
            printf("page cache - change n frames: Can not change the frame "
                   "size if there are still pinned pages!\n");
            exit(EXIT_FAILURE);
        }
    }

    free(pc->frames[0]->data);
    for (unsigned long i = 0; i < pc->n_frames; ++i) {
        page_destroy(pc->frames[i]);
    }
    free(pc->frames);

    for (file_kind i = 0; i < invalid; ++i) {
        if (i == catalogue) {
            dict_ul_ul_destroy(pc->page_map[i][0]);
            pc->page_map[i][0] = d_ul_ul_create();
            continue;
        }

        for (file_type j = 0; j < invalid_ft; ++j) {
            dict_ul_ul_destroy(pc->page_map[i][j]);
            pc->page_map[i][j] = d_ul_ul_create();
        }
    }

    queue_ul_destroy(pc->recently_referenced);
    pc->recently_referenced = q_ul_create();

    unsigned char* data = calloc(n_frames, PAGE_SIZE);

    if (!data) {
        // LCOV_EXCL_START
        printf("page cache - create: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    llist_ul_destroy(pc->free_frames);
    pc->free_frames = ll_ul_create();

    pc->n_frames = n_frames;
    pc->frames   = calloc(n_frames, sizeof(page*));
    for (unsigned long i = 0; i < n_frames; ++i) {
        pc->frames[i] = page_create(data + (PAGE_SIZE * i));
        llist_ul_append(pc->free_frames, i);
    }
}
