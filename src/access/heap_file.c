#include "access/heap_file.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "header_page.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"

#define NUM_SLOTS_PER_NODE                                                     \
    ((sizeof(node_t) / SLOT_SIZE) + (sizeof(node_t) % SLOT_SIZE != 0))

#define NUM_SLOTS_PER_REL                                                      \
    ((sizeof(relationship_t) / SLOT_SIZE)                                      \
     + (sizeof(relationship_t) % SLOT_SIZE != 0))

heap_file*
heap_file_create(page_cache* pc)
{
    if (!pc) {
        printf("heap file - create: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    heap_file* hf            = malloc(sizeof(heap_file));
    hf->cache                = pc;
    hf->last_alloc_node_slot = 0;
    hf->last_alloc_rel_slot  = 0;

    return hf;
}

void
heap_file_destroy(heap_file* hf)
{
    if (!hf) {
        printf("heap_file - destroy: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    free(hf);
}

unsigned long
next_free_slots(heap_file* hf, file_type ft)
{
    if (!hf || (ft != node_file && ft != relationship_file)) {
        printf("heap file - nex free slots: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    file_type hft = ft == node_file ? node_header : relationship_header;

    unsigned char n_slots =
          ft == node_file ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;

    unsigned long* prev_allocd_slots = ft == node_file
                                             ? &(hf->last_alloc_node_slot)
                                             : &(hf->last_alloc_rel_slot);

    unsigned long prev_allocd_slots_byte = sizeof(unsigned long)
                                           + (*prev_allocd_slots / CHAR_BIT)
                                           + (*prev_allocd_slots % CHAR_BIT);

    unsigned long prev_allocd_page_header_id =
          prev_allocd_slots_byte / PAGE_SIZE;

    page* prev_free_page_header =
          pin_page(hf->cache, prev_allocd_page_header_id, hft);

    unsigned short byte_offset = prev_allocd_slots_byte % PAGE_SIZE;

    unsigned char nxt_bits        = prev_free_page_header->data[byte_offset];
    page*         cur_page        = prev_free_page_header;
    unsigned long cur_page_id     = prev_allocd_page_header_id;
    unsigned long cur_byte_offset = byte_offset;
    unsigned long start_slot =
          *prev_allocd_slots - (*prev_allocd_slots % CHAR_BIT);
    unsigned long cur_slot               = start_slot;
    unsigned char consecutive_free_slots = 0;
    unsigned char first_free_slot;
    unsigned char write_mask = 1;

    for (size_t i = 1; i < n_slots; ++i) {
        write_mask = (write_mask << 1) | (1 << i);
    }

    do {
        for (unsigned char i = CHAR_BIT; i >= 0; --i) {
            if (((1 << i) & nxt_bits) == 0) {
                if (consecutive_free_slots == 0) {
                    first_free_slot = i;
                } else if ((cur_slot + i - sizeof(unsigned long) * CHAR_BIT)
                                 % SLOTS_PER_PAGE
                           == 0) {
                    consecutive_free_slots = 0;
                    first_free_slot        = i;
                }

                consecutive_free_slots++;

                if (consecutive_free_slots == n_slots) {
                    *prev_allocd_slots =
                          (cur_page_id * PAGE_SIZE - sizeof(unsigned long))
                                * CHAR_BIT
                          + cur_byte_offset * CHAR_BIT + first_free_slot;

                    write_bits(hf->cache,
                               cur_page,
                               cur_byte_offset,
                               i,
                               n_slots,
                               &write_mask);

                    unpin_page(hf->cache, prev_allocd_page_header_id, hft);

                    return ft == node_file ? hf->last_alloc_node_slot
                                           : hf->last_alloc_rel_slot;
                }
            } else {
                consecutive_free_slots = 0;
            }
        }

        if (cur_byte_offset >= PAGE_SIZE) {
            cur_byte_offset = 0;
            unpin_page(hf->cache, prev_allocd_page_header_id, hft);
            cur_page_id++;
            cur_page = pin_page(hf->cache, cur_page_id, hft);
        } else {
            cur_byte_offset++;
        }

        nxt_bits = cur_page->data[cur_byte_offset];

        cur_slot += CHAR_BIT;

    } while (cur_slot != start_slot);

    return new_page(hf->cache, ft) * SLOTS_PER_PAGE
           - (sizeof(unsigned long) * CHAR_BIT);
}

void
create_node(heap_file* hf, char* label)
{
    if (!hf || !label) {
        printf("heap file - create node: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    unsigned long node_slot = next_free_slots(hf, node_header);
    unsigned long node_id   = node_slot / NUM_SLOTS_PER_NODE;

    node_t* node = new_node();
    node->id     = node_id;
    strncpy(node->label, label, MAX_STR_LEN);

    unsigned long page_id   = node_id * sizeof(node_t) / PAGE_SIZE;
    page*         node_page = pin_page(hf->cache, page_id, node_file);

    node_write(node, node_page, node_slot % SLOTS_PER_PAGE);

    unpin_page(hf->cache, page_id, node_file);

    unsigned long header_id =
          (sizeof(unsigned long) + ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          / PAGE_SIZE;

    page* header = pin_page(hf->cache, header_id, node_header);

    unsigned short byte_offset =
          (sizeof(unsigned long) + ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          % PAGE_SIZE;
    unsigned char bit_offset = (node_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char used_bits  = UCHAR_MAX >> (CHAR_BIT - NUM_SLOTS_PER_NODE);

    write_bits(hf->cache, header, byte_offset, bit_offset, 1, &used_bits);

    unpin_page(hf->cache, header_id, node_header);
}

void
create_relationship(heap_file*    hf,
                    unsigned long node_from,
                    unsigned long node_to,
                    double        weight,
                    char*         label)
{}

node_t*
read_node(heap_file* hf, unsigned long node_id)
{}

relationship_t*
read_relationship(heap_file* hf, unsigned long rel_id)
{}

void
update_node(heap_file* hf, unsigned long target_node_id, node_t* node_to_write)
{}

void
update_relationship(heap_file*      hf,
                    unsigned long   target_rel_id,
                    relationship_t* rel_to_write)
{}

void
delete_node(heap_file* hf, unsigned long node_id)
{}

void
delete_relationship(heap_file* hf, unsigned long rel_id)
{}

// FIXME: Adjust record IDs
// FIXME move read & write btw. pages to read/write bits
void
swap_page(page_cache* pc, size_t fst, size_t snd, file_type ft)
{
    unsigned char* buf = malloc(sizeof(unsigned char) * PAGE_SIZE);

    size_t slots_per_page = PAGE_SIZE / SLOT_SIZE;

    size_t num_header_bits_fst = sizeof(unsigned long) + (fst * slots_per_page);
    size_t header_id_fst       = (num_header_bits_fst / CHAR_BIT) / PAGE_SIZE;
    unsigned short header_byte_offset_fst =
          (num_header_bits_fst / CHAR_BIT) % PAGE_SIZE;
    unsigned char header_bit_offset_fst = num_header_bits_fst % CHAR_BIT;

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

    size_t num_header_bits_snd = sizeof(unsigned long) + (snd * slots_per_page);
    size_t header_id_snd       = (num_header_bits_snd / CHAR_BIT) / PAGE_SIZE;
    unsigned short header_byte_offset_snd =
          (num_header_bits_snd / CHAR_BIT) % PAGE_SIZE;
    unsigned char header_bit_offset_snd = num_header_bits_snd % CHAR_BIT;

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

    page* fst_page   = pin_page(pc, fst, ft);
    page* snd_page   = pin_page(pc, snd, ft);
    page* fst_header = pin_page(pc, header_id_fst, ft - 1);
    page* snd_header = pin_page(pc, header_id_snd, ft - 1);

    page* fst_header_cont;
    page* snd_header_cont;
    if (bits_on_second_fst != 0) {
        fst_header_cont = pin_page(pc, header_id_fst + 1, ft - 1);
    }

    if (bits_on_second_snd != 0) {
        snd_header_cont = pin_page(pc, header_id_snd + 1, ft - 1);
    }

    memcpy(buf, fst_page->data, PAGE_SIZE);
    memcpy(fst_page->data, snd_page->data, PAGE_SIZE);
    memcpy(snd_page->data, buf, PAGE_SIZE);

    unsigned char* fst_header_bits = read_bits(fst_header,
                                               header_byte_offset_fst,
                                               header_bit_offset_fst,
                                               bits_on_first_fst);

    unsigned char* snd_header_bits = read_bits(snd_header,
                                               header_byte_offset_snd,
                                               header_bit_offset_snd,
                                               slots_per_page);

    if (bits_on_second_fst != 0) {
        unsigned char* fst_header_bits_cont = read_bits(
              fst_header_cont, header_id_fst + 1, 0, bits_on_second_fst);
        concat_bit_arrays(fst_header_bits,
                          fst_header_bits_cont,
                          bits_on_first_fst,
                          bits_on_second_fst);
    }

    if (bits_on_second_snd != 0) {
        unsigned char* snd_header_bits_cont = read_bits(
              snd_header_cont, header_id_snd + 1, 0, bits_on_second_snd);

        concat_bit_arrays(snd_header_bits,
                          snd_header_bits_cont,
                          bits_on_first_snd,
                          bits_on_second_snd);
    }

    unsigned char** split;
    if (bits_on_second_fst != 0) {
        split = split_bit_array(
              snd_header_bits, slots_per_page, bits_on_first_fst);

        write_bits(fst_header_cont, 0, 0, bits_on_first_snd, split[1]);
        write_bits(fst_header,
                   header_byte_offset_fst,
                   header_bit_offset_fst,
                   bits_on_first_fst,
                   split[0]);

        free(split[0]);
        free(split[1]);
    } else {
        write_bits(fst_header,
                   header_byte_offset_fst,
                   header_bit_offset_fst,
                   bits_on_first_fst,
                   snd_header_bits);
    }

    if (bits_on_second_snd != 0) {
        split = split_bit_array(
              fst_header_bits, slots_per_page, bits_on_second_fst);

        write_bits(snd_header_cont, 0, 0, bits_on_second_snd, split[1]);
        write_bits(snd_header,
                   header_byte_offset_snd,
                   header_bit_offset_snd,
                   bits_on_second_fst,
                   split[0]);

        free(split[0]);
        free(split[1]);
    } else {
        write_bits(snd_header,
                   header_byte_offset_snd,
                   header_bit_offset_snd,
                   bits_on_second_fst,
                   fst_header_bits);
    }

    fst_page->dirty   = true;
    snd_page->dirty   = true;
    fst_header->dirty = true;
    snd_header->dirty = true;

    if (bits_on_second_fst != 0) {
        fst_header_cont->dirty = true;
        unpin_page(pc, header_id_fst + 1, ft - 1);
    }

    if (bits_on_second_snd != 0) {
        snd_header_cont->dirty = true;
        unpin_page(pc, header_id_snd + 1, ft - 1);
    }

    unpin_page(pc, header_id_fst, ft - 1);
    unpin_page(pc, header_id_snd, ft - 1);
    unpin_page(pc, snd, ft);
    unpin_page(pc, fst, ft);

    free(buf);
    free(fst_header);
    free(snd_header);
}

