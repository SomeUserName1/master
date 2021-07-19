#include "page_cache.h"

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

    free(pc);
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

void
swap_page(page_cache* pc, size_t fst, size_t snd, file_type ft)
{
    unsigned char* buf = malloc(sizeof(unsigned char) * PAGE_SIZE);

    size_t slots_per_page = PAGE_SIZE / SLOT_SIZE;

    size_t header_bits_fst = sizeof(unsigned long) + (fst * slots_per_page);
    size_t header_page_fst = (header_bits_fst / CHAR_BIT) / PAGE_SIZE;
    unsigned short header_byte_offset_fst =
          (header_bits_fst / CHAR_BIT) % PAGE_SIZE;
    unsigned char header_bit_offset_fst = header_bits_fst % CHAR_BIT;

    unsigned short bits_on_first_fst;
    unsigned short bits_on_second_fst;
    if (header_byte_offset_fst == PAGE_SIZE - 1
        && header_bit_offset_fst + slots_per_page > CHAR_BIT) {
        bits_on_first_fst  = CHAR_BIT - header_bit_offset_fst;
        bits_on_second_fst = slots_per_page - bits_on_first_fst;
    } else {
        bits_on_first_fst  = slots_per_page;
        bits_on_second_fst = 0;
    }

    size_t header_bits_snd = sizeof(unsigned long) + (snd * slots_per_page);
    size_t header_page_snd = (header_bits_snd / CHAR_BIT) / PAGE_SIZE;
    unsigned short header_byte_offset_snd =
          (header_bits_snd / CHAR_BIT) % PAGE_SIZE;
    unsigned char header_bit_offset_snd = header_bits_snd % CHAR_BIT;

    unsigned short bits_on_first_snd;
    unsigned short bits_on_second_snd;
    if (header_byte_offset_snd == PAGE_SIZE - 1
        && header_bit_offset_snd + slots_per_page > CHAR_BIT) {
        bits_on_first_snd  = CHAR_BIT - header_bit_offset_snd;
        bits_on_second_snd = slots_per_page - bits_on_first_snd;
    } else {
        bits_on_first_snd  = slots_per_page;
        bits_on_second_snd = 0;
    }

    pin_page(pc, fst, ft);
    pin_page(pc, snd, ft);
    pin_page(pc, header_page_fst, ft - 1);
    pin_page(pc, header_page_snd, ft - 1);

    if (bits_on_second_fst != 0) {
        pin_page(pc, header_page_fst + 1, ft - 1);
    }

    if (bits_on_second_snd != 0) {
        pin_page(pc, header_page_snd + 1, ft - 1);
    }

    size_t frame_no[SWAP_MAX_NUM_PINNED_PAGES];
    frame_no[0] = dict_ul_ul_get_direct(pc->page_map[ft], fst);
    frame_no[1] = dict_ul_ul_get_direct(pc->page_map[ft], snd);
    frame_no[2] = dict_ul_ul_get_direct(pc->page_map[ft - 1], header_page_fst);
    frame_no[3] = dict_ul_ul_get_direct(pc->page_map[ft - 1], header_page_snd);

    if (bits_on_second_fst) {
        frame_no[4] =
              dict_ul_ul_get_direct(pc->page_map[ft - 1], header_page_fst + 1);
    }

    if (bits_on_second_snd) {
        frame_no[SWAP_MAX_NUM_PINNED_PAGES - 1] =
              dict_ul_ul_get_direct(pc->page_map[ft - 1], header_page_snd + 1);
    }

    memcpy(buf, pc->cache[frame_no[0]]->data, PAGE_SIZE);
    memcpy(pc->cache[frame_no[0]]->data,
           pc->cache[frame_no[1]]->data,
           PAGE_SIZE);
    memcpy(pc->cache[frame_no[1]]->data, buf, PAGE_SIZE);

    unsigned char* fst_header = read_bits(pc->cache[frame_no[2]],
                                          header_byte_offset_fst,
                                          header_bit_offset_fst,
                                          bits_on_first_fst);

    unsigned char* snd_header = read_bits(pc->cache[frame_no[3]],
                                          header_byte_offset_snd,
                                          header_bit_offset_snd,
                                          slots_per_page);

    if (bits_on_second_fst != 0) {
        unsigned char* fst_header_cont = read_bits(pc->cache[frame_no[4]],
                                                   header_page_fst + 1,
                                                   0,
                                                   bits_on_second_fst);
        concat_bit_arrays(fst_header,
                          fst_header_cont,
                          bits_on_first_fst,
                          bits_on_second_fst);
    }

    if (bits_on_second_snd != 0) {
        unsigned char* snd_header_cont =
              read_bits(pc->cache[frame_no[SWAP_MAX_NUM_PINNED_PAGES - 1]],
                        header_page_snd + 1,
                        0,
                        bits_on_second_snd);

        concat_bit_arrays(snd_header,
                          snd_header_cont,
                          bits_on_first_snd,
                          bits_on_second_snd);
    }

    unsigned char** split;
    if (bits_on_second_fst != 0) {
        split = split_bit_array(snd_header, slots_per_page, bits_on_first_fst);

        write_bits(pc->cache[frame_no[4]], 0, 0, bits_on_first_snd, split[1]);
        write_bits(pc->cache[frame_no[2]],
                   header_byte_offset_fst,
                   header_bit_offset_fst,
                   bits_on_first_fst,
                   split[0]);

        free(split[0]);
        free(split[1]);
    } else {
        write_bits(pc->cache[frame_no[2]],
                   header_byte_offset_fst,
                   header_bit_offset_fst,
                   bits_on_first_fst,
                   snd_header);
    }

    if (bits_on_second_snd != 0) {
        split = split_bit_array(fst_header, slots_per_page, bits_on_second_fst);

        write_bits(pc->cache[frame_no[SWAP_MAX_NUM_PINNED_PAGES - 1]],
                   0,
                   0,
                   bits_on_second_snd,
                   split[1]);
        write_bits(pc->cache[frame_no[3]],
                   header_byte_offset_snd,
                   header_bit_offset_snd,
                   bits_on_second_fst,
                   split[0]);

        free(split[0]);
        free(split[1]);
    } else {
        write_bits(pc->cache[frame_no[3]],
                   header_byte_offset_snd,
                   header_bit_offset_snd,
                   bits_on_second_fst,
                   fst_header);
    }

    pc->cache[frame_no[0]]->dirty = true;
    pc->cache[frame_no[1]]->dirty = true;
    pc->cache[frame_no[2]]->dirty = true;
    pc->cache[frame_no[3]]->dirty = true;

    if (bits_on_second_fst != 0) {
        pc->cache[frame_no[4]]->dirty = true;
        unpin_page(pc, header_page_fst + 1, ft - 1);
    }

    if (bits_on_second_snd != 0) {
        pc->cache[frame_no[SWAP_MAX_NUM_PINNED_PAGES - 1]]->dirty = true;
        unpin_page(pc, header_page_snd + 1, ft - 1);
    }

    unpin_page(pc, header_page_fst, ft - 1);
    unpin_page(pc, header_page_snd, ft - 1);
    unpin_page(pc, snd, ft);
    unpin_page(pc, fst, ft);

    free(buf);
    free(fst_header);
    free(snd_header);
}

