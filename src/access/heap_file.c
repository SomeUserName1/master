#include "access/heap_file.h"

#include <math.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
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

    heap_file* hf = malloc(sizeof(heap_file));
    hf->cache     = pc;

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

    write_bits(header, byte_offset, bit_offset, 1, &used_bits);

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

