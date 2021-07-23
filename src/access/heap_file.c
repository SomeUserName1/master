#include "access/heap_file.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "header_page.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"

#define NUM_SLOTS_PER_NODE                                                     \
    ((ON_DISK_NODE_SIZE / SLOT_SIZE) + (ON_DISK_NODE_SIZE % SLOT_SIZE != 0))

#define NUM_SLOTS_PER_REL                                                      \
    ((ON_DISK_REL_SIZE / SLOT_SIZE) + (ON_DISK_REL_SIZE % SLOT_SIZE != 0))

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

    update_node(hf, node);

    unsigned long header_id =
          (sizeof(unsigned long) + ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          / PAGE_SIZE;

    page* header = pin_page(hf->cache, header_id, node_header);

    unsigned short byte_offset =
          (sizeof(unsigned long) + ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          % PAGE_SIZE;
    unsigned char bit_offset = (node_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char used_bits  = UCHAR_MAX >> (CHAR_BIT - NUM_SLOTS_PER_NODE);

    write_bits(hf->cache,
               header,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_NODE,
               &used_bits);

    unpin_page(hf->cache, header_id, node_header);
}

void
create_relationship(heap_file*    hf,
                    unsigned long from_node_id,
                    unsigned long to_node_id,
                    double        weight,
                    char*         label)
{
    if (!hf || from_node_id == UNINITIALIZED_LONG
        || to_node_id == UNINITIALIZED_LONG || weight == UNINITIALIZED_WEIGHT
        || !label) {
        printf("heap file - create relationship: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    unsigned long rel_slot = next_free_slots(hf, relationship_header);
    unsigned long rel_id   = rel_slot / NUM_SLOTS_PER_NODE;

    relationship_t* rel = new_relationship();
    rel->id             = rel_id;
    strncpy(rel->label, label, MAX_STR_LEN);

    node_t* from_node = read_node(hf, from_node_id);
    node_t* to_node   = read_node(hf, from_node_id);

    relationship_t* first_rel_from;
    relationship_t* last_rel_from;
    relationship_t* first_rel_to;
    relationship_t* last_rel_to;
    unsigned long   temp_id;

    if (from_node->first_relationship == UNINITIALIZED_LONG) {
        first_rel_from = rel;
        last_rel_from  = rel;
    } else {
        first_rel_from = read_relationship(hf, from_node->first_relationship);
        temp_id        = from_node->id == first_rel_from->source_node
                               ? first_rel_from->prev_rel_source
                               : first_rel_from->prev_rel_target;

        last_rel_from = read_relationship(hf, temp_id);
    }

    if (to_node->first_relationship == UNINITIALIZED_LONG) {
        first_rel_to = rel;
        last_rel_to  = rel;
    } else {
        first_rel_to = read_relationship(hf, to_node->first_relationship);
        temp_id      = from_node->id == first_rel_to->source_node
                             ? first_rel_to->prev_rel_source
                             : first_rel_to->prev_rel_target;

        last_rel_to = read_relationship(hf, temp_id);
    }

    rel->prev_rel_source = last_rel_from->id;
    rel->next_rel_source = first_rel_from->id;
    rel->prev_rel_target = last_rel_to->id;
    rel->prev_rel_target = first_rel_to->id;

    // Adjust next pointer in source node's previous relation
    if (last_rel_from->source_node == from_node_id) {
        last_rel_from->next_rel_source = rel->id;
    }
    if (last_rel_from->target_node == from_node_id) {
        last_rel_from->next_rel_target = rel->id;
    }
    // Adjust next pointer in target node's previous relation
    if (last_rel_to->source_node == to_node_id) {
        last_rel_to->next_rel_source = rel->id;
    } else {
        last_rel_to->next_rel_target = rel->id;
    }

    // Adjust previous pointer in source node's next relation
    if (first_rel_from->source_node == from_node_id) {
        first_rel_from->prev_rel_source = rel->id;
    } else {
        first_rel_from->prev_rel_target = rel->id;
    }
    // Adjust previous pointer in target node's next relation
    if (first_rel_to->source_node == to_node_id) {
        first_rel_to->prev_rel_source = rel->id;
    } else {
        first_rel_to->prev_rel_target = rel->id;
    }

    // Set the first relationship pointer, if the inserted rel is the first rel
    if (from_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_source(rel);
        from_node->first_relationship = rel->id;
    } else {
        relationship_set_first_target(rel);
        to_node->first_relationship = rel->id;
    }

    update_relationship(hf, rel);
    update_relationship(hf, first_rel_from);
    update_relationship(hf, last_rel_from);
    update_relationship(hf, first_rel_to);
    update_relationship(hf, last_rel_to);

    unsigned long header_id =
          (sizeof(unsigned long) + ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          / PAGE_SIZE;

    page* header = pin_page(hf->cache, header_id, relationship_header);

    unsigned short byte_offset =
          (sizeof(unsigned long) + ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          % PAGE_SIZE;
    unsigned char bit_offset = (rel_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char used_bits  = UCHAR_MAX >> (CHAR_BIT - NUM_SLOTS_PER_NODE);

    write_bits(hf->cache,
               header,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_REL,
               &used_bits);

    unpin_page(hf->cache, header_id, node_header);
}

node_t*
read_node(heap_file* hf, unsigned long node_id)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        printf("heap file - read node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id   = node_id * ON_DISK_NODE_SIZE / PAGE_SIZE;
    page*         node_page = pin_page(hf->cache, page_id, node_file);

    node_t* node = new_node();
    node->id     = node_id;
    node_read(node, node_page);

    unpin_page(hf->cache, page_id, node_file);

    return node;
}

relationship_t*
read_relationship(heap_file* hf, unsigned long rel_id)
{
    if (!hf || rel_id == UNINITIALIZED_LONG) {
        printf("heap file - read relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id  = rel_id * ON_DISK_REL_SIZE / PAGE_SIZE;
    page*         rel_page = pin_page(hf->cache, page_id, relationship_file);

    relationship_t* rel = new_relationship();
    rel->id             = rel_id;
    relationship_read(rel, rel_page);

    unpin_page(hf->cache, page_id, relationship_file);

    return rel;
}

void
update_node(heap_file* hf, node_t* node_to_write)
{
    if (!hf || !node_to_write || node_to_write->id == UNINITIALIZED_LONG) {
        printf("heap file - update node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id   = node_to_write->id * ON_DISK_REL_SIZE / PAGE_SIZE;
    page*         node_page = pin_page(hf->cache, page_id, node_file);

    node_write(node_to_write, node_page);

    unpin_page(hf->cache, page_id, relationship_file);
}

void
update_relationship(heap_file* hf, relationship_t* rel_to_write)
{
    if (!hf || !rel_to_write || rel_to_write->id == UNINITIALIZED_LONG
        || rel_to_write->source_node == UNINITIALIZED_LONG
        || rel_to_write->target_node == UNINITIALIZED_LONG
        || rel_to_write->weight == UNINITIALIZED_WEIGHT
        || rel_to_write->flags == UNINITIALIZED_BYTE) {
        printf("heap file - update node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id   = rel_to_write->id * ON_DISK_REL_SIZE / PAGE_SIZE;
    page*         node_page = pin_page(hf->cache, page_id, node_file);

    node_t* node = new_node();

    node_write(node, node_page);

    unpin_page(hf->cache, page_id, relationship_file);
}

void
delete_node(heap_file* hf, unsigned long node_id)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        printf("heap file - delete node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = read_node(hf, node_id);

    if (node->first_relationship != UNINITIALIZED_LONG) {
        relationship_t* rel = read_relationship(hf, node->first_relationship);
        unsigned long   prev_rel_id = node_id == rel->source_node
                                            ? rel->prev_rel_source
                                            : rel->prev_rel_target;

        while (prev_rel_id != UNINITIALIZED_LONG) {
            rel         = read_relationship(hf, prev_rel_id);
            prev_rel_id = node_id == rel->source_node ? rel->prev_rel_source
                                                      : rel->prev_rel_target;
            delete_relationship(hf, rel->id);
        }
    }

    node     = new_node();
    node->id = node_id;
    memset(node->label, 0, MAX_STR_LEN);

    update_node(hf, node);

    unsigned long header_id =
          (sizeof(unsigned long) + ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          / PAGE_SIZE;

    page* header = pin_page(hf->cache, header_id, node_header);

    unsigned short byte_offset =
          (sizeof(unsigned long) + ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          % PAGE_SIZE;
    unsigned char bit_offset  = (node_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char unused_bits = 0;

    write_bits(hf->cache,
               header,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_NODE,
               &unused_bits);

    unpin_page(hf->cache, header_id, node_header);
}

void
delete_relationship(heap_file* hf, unsigned long rel_id)
{
    if (!hf || rel_id == UNINITIALIZED_LONG) {
        printf("heap file - delete relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    relationship_t* rel = read_relationship(hf, rel_id);

    // Adjust next pointer in source node's previous relation
    relationship_t* prev_rel_from = read_relationship(hf, rel->prev_rel_source);
    if (prev_rel_from->source_node == rel->source_node) {
        prev_rel_from->next_rel_source = rel->next_rel_source;
    } else {
        prev_rel_from->next_rel_target = rel->next_rel_source;
    }

    // Adjust next pointer in target node's previous relation
    relationship_t* prev_rel_to = read_relationship(hf, rel->prev_rel_target);
    if (prev_rel_to->source_node == rel->target_node) {
        prev_rel_to->next_rel_source = rel->next_rel_target;
    } else {
        prev_rel_to->next_rel_target = rel->next_rel_target;
    }

    // Adjust previous pointer in source node's next relation
    relationship_t* next_rel_from = read_relationship(hf, rel->next_rel_source);
    if (next_rel_from->source_node == rel->source_node) {
        next_rel_from->prev_rel_source = rel->prev_rel_source;
    } else {
        next_rel_from->prev_rel_target = rel->prev_rel_source;
    }

    // Adjust previous pointer in target node's next relation
    relationship_t* next_rel_to = read_relationship(hf, rel->next_rel_target);
    if (next_rel_to->source_node == rel->target_node) {
        next_rel_to->prev_rel_source = rel->prev_rel_target;
    } else {
        next_rel_to->prev_rel_target = rel->prev_rel_target;
    }

    update_relationship(hf, prev_rel_from);
    update_relationship(hf, prev_rel_to);
    update_relationship(hf, next_rel_from);
    update_relationship(hf, next_rel_to);

    node_t* node;
    if ((rel->flags & FIRST_REL_SOURCE_FLAG) != 0) {
        node                     = read_node(hf, rel->source_node);
        node->first_relationship = rel->next_rel_source;
        update_node(hf, node);

        rel = read_relationship(hf, rel->next_rel_source);
        relationship_set_first_source(rel);
        update_relationship(hf, rel);
    }

    if ((rel->flags & FIRST_REL_TARGET_FLAG) != 0) {
        node                     = read_node(hf, rel->target_node);
        node->first_relationship = rel->next_rel_target;
        update_node(hf, node);

        rel = read_relationship(hf, rel->next_rel_target);
        relationship_set_first_target(rel);
        update_relationship(hf, rel);
    }

    rel     = new_relationship();
    rel->id = rel_id;

    update_relationship(hf, rel);

    unsigned long header_id =
          (sizeof(unsigned long) + ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          / PAGE_SIZE;

    page* header = pin_page(hf->cache, header_id, relationship_header);

    unsigned short byte_offset =
          (sizeof(unsigned long) + ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT))
          % PAGE_SIZE;
    unsigned char bit_offset  = (rel_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char unused_bits = 0;

    write_bits(hf->cache,
               header,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_REL,
               &unused_bits);

    unpin_page(hf->cache, header_id, node_header);
}

void
move_node(heap_file* hf, unsigned long id, unsigned long to_id)
{
    if (!hf || id == UNINITIALIZED_LONG) {
        printf("heap file - move node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = read_node(hf, id);

    unsigned long   rel_id = node->first_relationship;
    relationship_t* rel;

    do {
        rel = read_relationship(hf, rel_id);

        if (rel->source_node == id) {
            rel_id           = rel->next_rel_source;
            rel->source_node = to_id;
        } else {
            rel_id           = rel->next_rel_target;
            rel->target_node = to_id;
        }

        update_relationship(hf, rel);
    } while (rel_id != node->first_relationship);
}

void
move_relationship(heap_file* hf, unsigned long id, unsigned long to_id)
{
    if (!hf || id == UNINITIALIZED_LONG) {
        printf("heap file - move relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    // Go through both incidence lists and check if the relationship to be moved
    // appears there. if so, adjust the id
    relationship_t* rel = read_relationship(hf, id);

    // Adjust next pointer in source node's previous relation
    relationship_t* prev_rel_from = read_relationship(hf, rel->prev_rel_source);
    if (prev_rel_from->source_node == rel->source_node) {
        prev_rel_from->next_rel_source = to_id;
    } else {
        prev_rel_from->next_rel_target = to_id;
    }

    // Adjust next pointer in target node's previous relation
    relationship_t* prev_rel_to = read_relationship(hf, rel->prev_rel_target);
    if (prev_rel_to->source_node == rel->target_node) {
        prev_rel_to->next_rel_source = to_id;
    } else {
        prev_rel_to->next_rel_target = to_id;
    }

    // Adjust previous pointer in source node's next relation
    relationship_t* next_rel_from = read_relationship(hf, rel->next_rel_source);
    if (next_rel_from->source_node == rel->source_node) {
        next_rel_from->prev_rel_source = to_id;
    } else {
        next_rel_from->prev_rel_target = to_id;
    }

    // Adjust previous pointer in target node's next relation
    relationship_t* next_rel_to = read_relationship(hf, rel->next_rel_target);
    if (next_rel_to->source_node == rel->target_node) {
        next_rel_to->prev_rel_source = to_id;
    } else {
        next_rel_to->prev_rel_target = to_id;
    }

    update_relationship(hf, prev_rel_from);
    update_relationship(hf, prev_rel_to);
    update_relationship(hf, next_rel_from);
    update_relationship(hf, next_rel_to);

    // adjust the id in the nodes first relationship fields if neccessary
    node_t* node;
    if ((rel->flags & FIRST_REL_SOURCE_FLAG) != 0) {
        node                     = read_node(hf, rel->source_node);
        node->first_relationship = to_id;
        update_node(hf, node);
    }

    if ((rel->flags & FIRST_REL_TARGET_FLAG) != 0) {
        node                     = read_node(hf, rel->target_node);
        node->first_relationship = to_id;
        update_node(hf, node);
    }
}

void
swap_page(heap_file* hf, size_t fst, size_t snd, file_type ft)
{
    if (!hf || fst >= MAX_PAGE_NO || snd > MAX_PAGE_NO
        || (ft != node_file && ft != relationship_file)) {
        printf("heap file - swap_pages: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    size_t num_header_bits_fst = sizeof(unsigned long) + (fst * SLOTS_PER_PAGE);

    size_t header_id_fst = (num_header_bits_fst / CHAR_BIT) / PAGE_SIZE;

    unsigned short header_byte_offset_fst =
          (num_header_bits_fst / CHAR_BIT) % PAGE_SIZE;

    unsigned char header_bit_offset_fst = num_header_bits_fst % CHAR_BIT;

    size_t num_header_bits_snd = sizeof(unsigned long) + (snd * SLOTS_PER_PAGE);

    size_t header_id_snd = (num_header_bits_snd / CHAR_BIT) / PAGE_SIZE;

    unsigned short header_byte_offset_snd =
          (num_header_bits_snd / CHAR_BIT) % PAGE_SIZE;

    unsigned char header_bit_offset_snd = num_header_bits_snd % CHAR_BIT;

    page* fst_page   = pin_page(hf->cache, fst, ft);
    page* snd_page   = pin_page(hf->cache, snd, ft);
    page* fst_header = pin_page(hf->cache, header_id_fst, ft - 1);
    page* snd_header = pin_page(hf->cache, header_id_snd, ft - 1);

    unsigned char* fst_header_bits = read_bits(hf->cache,
                                               fst_header,
                                               header_byte_offset_fst,
                                               header_bit_offset_fst,
                                               SLOTS_PER_PAGE);

    unsigned char* snd_header_bits = read_bits(hf->cache,
                                               snd_header,
                                               header_byte_offset_snd,
                                               header_bit_offset_snd,
                                               SLOTS_PER_PAGE);

    unsigned char n_slots =
          ft == node_file ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;

    unsigned char slot_used_mask = 1;

    for (size_t i = 1; i < n_slots; ++i) {
        slot_used_mask = (slot_used_mask << 1) | (1 << i);
    }

    unsigned long id    = UNINITIALIZED_LONG;
    unsigned long to_id = UNINITIALIZED_LONG;
    for (size_t i = 0; i < SLOTS_PER_PAGE; i += n_slots) {
        if (compare_bits(fst_header_bits, slot_used_mask, i)) {
            id    = fst * SLOTS_PER_PAGE + i;
            to_id = snd * SLOTS_PER_PAGE + i;

            if (ft == node_file) {
                move_node(hf, id, to_id);
            } else {
                move_relationship(hf, id, to_id);
            }
        }
    }

    id = UNINITIALIZED_LONG;
    for (size_t i = 0; i < SLOTS_PER_PAGE; i += n_slots) {
        if (compare_bits(snd_header_bits, slot_used_mask, i)) {
            id    = snd * SLOTS_PER_PAGE + i;
            to_id = fst * SLOTS_PER_PAGE + i;

            if (ft == node_file) {
                move_node(hf, id, to_id);

            } else {
                move_relationship(hf, id, to_id);
            }
        }
    }

    unsigned char* buf = malloc(sizeof(unsigned char) * PAGE_SIZE);
    memcpy(buf, fst_page->data, PAGE_SIZE);
    memcpy(fst_page->data, snd_page->data, PAGE_SIZE);
    memcpy(snd_page->data, buf, PAGE_SIZE);

    write_bits(hf->cache,
               fst_header,
               header_byte_offset_fst,
               header_bit_offset_fst,
               SLOTS_PER_PAGE,
               snd_header_bits);

    write_bits(hf->cache,
               snd_header,
               header_byte_offset_snd,
               header_bit_offset_snd,
               SLOTS_PER_PAGE,
               fst_header_bits);

    fst_page->dirty   = true;
    snd_page->dirty   = true;
    fst_header->dirty = true;
    snd_header->dirty = true;

    unpin_page(hf->cache, header_id_fst, ft - 1);
    unpin_page(hf->cache, header_id_snd, ft - 1);
    unpin_page(hf->cache, snd, ft);
    unpin_page(hf->cache, fst, ft);

    free(buf);
    free(fst_header);
    free(snd_header);
}

