#include "access/heap_file.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "access/header_page.h"
#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"

#define NUM_SLOTS_PER_NODE                                                     \
    ((ON_DISK_NODE_SIZE / SLOT_SIZE) + (ON_DISK_NODE_SIZE % SLOT_SIZE != 0))

#define NUM_SLOTS_PER_REL                                                      \
    ((ON_DISK_REL_SIZE / SLOT_SIZE) + (ON_DISK_REL_SIZE % SLOT_SIZE != 0))

heap_file*
heap_file_create(page_cache* pc, const char* log_path)
{
    if (!pc || !log_path) {
        printf("heap file - create: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    heap_file* hf            = malloc(sizeof(heap_file));
    hf->cache                = pc;
    hf->last_alloc_node_slot = 0;
    hf->last_alloc_rel_slot  = 0;

    array_list_node* nodes = get_nodes(hf);
    hf->n_nodes            = array_list_node_size(nodes);
    array_list_node_destroy(nodes);

    array_list_relationship* rels = get_relationships(hf);
    hf->n_rels                    = array_list_relationship_size(rels);
    array_list_relationship_destroy(rels);

    FILE* log_file = fopen(log_path, "w+");

    if (log_file == NULL) {
        printf("heap file - create: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }

    hf->log_file = log_file;

    return hf;
}

void
heap_file_destroy(heap_file* hf)
{
    if (!hf) {
        printf("heap_file - destroy: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }
    fclose(hf->log_file);
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

                    unpin_page(hf->cache, cur_page_id, hft);

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

unsigned long
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

    free(node);

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

    hf->n_nodes++;

    return node_id;
}

unsigned long
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
        update_node(hf, from_node);
    }

    if (to_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_target(rel);
        to_node->first_relationship = rel->id;
        update_node(hf, to_node);
    }

    update_relationship(hf, rel);
    update_relationship(hf, first_rel_from);
    update_relationship(hf, last_rel_from);
    update_relationship(hf, first_rel_to);
    update_relationship(hf, last_rel_to);

    free(rel);
    free(first_rel_from);
    free(last_rel_from);
    free(first_rel_to);
    free(last_rel_to);

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

    hf->n_rels++;

    return rel_id;
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
#ifdef VERBOSE
    fprintf(hf->log_file, "Read_Node %lu\n", node_id);
#endif

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

#ifdef VERBOSE
    fprintf(hf->log_file, "read_rel %lu\n", rel_id);
#endif

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

#ifdef VERBOSE
    fprintf(hf->log_file, "update_node %lu\n", node_to_write->id);
#endif
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

#ifdef VERBOSE
    fprintf(hf->log_file, "update_rel %lu\n", rel_to_write->id);
#endif
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

#ifdef VERBOSE
    fprintf(hf->log_file, "delete_node %lu\n", node_id);
#endif

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

    hf->n_nodes--;
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

#ifdef VERBOSE
    fprintf(hf->log_file, "delete_rel %lu\n", rel_id);
#endif

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

    hf->n_rels--;
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

    unsigned char slot_used_mask = UCHAR_MAX >> (CHAR_BIT - n_slots);

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

#ifdef VERBOSE
    fprintf(hf->log_file,
            "swap_%s_pages %lu\nSwap_%s_pages %lu\n",
            ft == node_file ? "node" : "rel",
            fst,
            snd);
#endif

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

array_list_node*
get_nodes(heap_file* hf)
{
    if (!hf) {
        printf("heap file operators - get nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t         header_id = 0;
    page*          header    = pin_page(hf->cache, header_id, node_header);
    bool           first     = true;
    unsigned long  bit_pos_h;
    unsigned short byte_pos_h;

    unsigned long n_slots = read_ulong(header, 0);

    array_list_node* result = al_node_create();
    node_t*          node;

    unsigned char slot_used_mask = UCHAR_MAX >> (CHAR_BIT - NUM_SLOTS_PER_NODE);

    unsigned char* slot_used;
    for (size_t i = 0; i < n_slots; i += NUM_SLOTS_PER_NODE) {
        // for the first page we need to take the unsigned long at the start
        // into account that stores the amount of slots
        byte_pos_h = first ? (sizeof(unsigned long) + i / CHAR_BIT) % PAGE_SIZE
                           : (i / CHAR_BIT) % PAGE_SIZE;

        // Read the corresponding header bits for the slots
        slot_used = read_bits(
              hf->cache, header, byte_pos_h, i % CHAR_BIT, NUM_SLOTS_PER_NODE);

        // If the slot is used, load the node and append it to the resulting
        // array list
        if (compare_bits(slot_used, slot_used_mask, i)) {
            node = read_node(hf, i);
            array_list_node_append(result, node);
        }

        free(slot_used);

        // if header page boundary is reached, unpin the current one and pin the
        // next one
        bit_pos_h = first ? sizeof(unsigned long) * CHAR_BIT + i : i;
        if (bit_pos_h % SLOTS_PER_PAGE
            > (bit_pos_h + NUM_SLOTS_PER_NODE) % SLOTS_PER_PAGE) {
            first = false;
            unpin_page(hf->cache, header_id, node_header);
            ++header_id;
            pin_page(hf->cache, header_id, node_header);
        }
    }

    return result;
}

array_list_relationship*
get_relationships(heap_file* hf)
{
    if (!hf) {
        printf("heap file operators - get relationships: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t         header_id = 0;
    page*          header = pin_page(hf->cache, header_id, relationship_header);
    bool           first  = true;
    unsigned long  bit_pos_h;
    unsigned short byte_pos_h;

    unsigned long n_slots = read_ulong(header, 0);

    array_list_relationship* result = al_rel_create();
    relationship_t*          rel;

    unsigned char slot_used_mask = UCHAR_MAX >> (CHAR_BIT - NUM_SLOTS_PER_REL);

    unsigned char* slot_used;
    for (size_t i = 0; i < n_slots; i += NUM_SLOTS_PER_REL) {
        byte_pos_h = first ? (sizeof(unsigned long) + i / CHAR_BIT) % PAGE_SIZE
                           : (i / CHAR_BIT) % PAGE_SIZE;
        slot_used  = read_bits(
              hf->cache, header, byte_pos_h, i % CHAR_BIT, NUM_SLOTS_PER_REL);

        if (compare_bits(slot_used, slot_used_mask, i)) {

            rel = read_relationship(hf, i);
            array_list_relationship_append(result, rel);
        }

        free(slot_used);

        // if header page boundary is readed, unpin the current one and pin the
        // next one
        bit_pos_h = first ? sizeof(unsigned long) * CHAR_BIT + i : i;
        if (bit_pos_h % SLOTS_PER_PAGE
            > (bit_pos_h + NUM_SLOTS_PER_NODE) % SLOTS_PER_PAGE) {
            first = false;
            unpin_page(hf->cache, header_id, node_header);
            ++header_id;
            pin_page(hf->cache, header_id, node_header);
        }
    }

    return result;
}

unsigned long
next_relationship_id(heap_file*      hf,
                     unsigned long   node_id,
                     relationship_t* rel,
                     direction_t     direction)
{
    if (!hf || node_id == UNINITIALIZED_LONG || !rel) {
        printf("heap_file - next relationship id: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long start_rel_id = rel->id;
    unsigned long rel_id = node_id == rel->source_node ? rel->next_rel_source
                                                       : rel->next_rel_target;

    do {
        rel = read_relationship(hf, rel_id);

        if (rel_id != start_rel_id
            && ((rel->source_node == node_id && direction != INCOMING)
                || (rel->target_node == node_id && direction != OUTGOING))) {
            return rel->id;
        }
        rel_id = node_id == rel->source_node ? rel->next_rel_source
                                             : rel->next_rel_target;

    } while (rel_id != start_rel_id);

    return UNINITIALIZED_LONG;
}

array_list_relationship*
expand(heap_file* hf, unsigned long node_id, direction_t direction)
{
    if (!hf) {
        printf("heap file - expand: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = read_node(hf, node_id);

    array_list_relationship* result = al_rel_create();
    unsigned long            rel_id = node->first_relationship;

    if (rel_id == UNINITIALIZED_LONG) {
        return result;
    }

    relationship_t* rel = read_relationship(hf, rel_id);
    unsigned long   start_id;

    if ((rel->source_node == node_id && direction != INCOMING)
        || (rel->target_node == node_id && direction != OUTGOING)) {
        start_id = rel_id;
    } else {
        rel_id   = next_relationship_id(hf, node_id, rel, direction);
        start_id = rel_id;
    }

    while (rel_id != UNINITIALIZED_LONG) {
        rel = read_relationship(hf, rel_id);
        array_list_relationship_append(result, rel);
        rel_id = next_relationship_id(hf, node->id, rel, direction);

        if (rel_id == start_id) {
            return result;
        }
    }

    return result;
}

relationship_t*
contains_relationship_from_to(heap_file*    hf,
                              unsigned long node_from,
                              unsigned long node_to,
                              direction_t   direction)
{
    if (!hf || node_from == UNINITIALIZED_LONG
        || node_to == UNINITIALIZED_LONG) {
        printf("heap file - contains relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    relationship_t* rel;
    node_t*         source_node = read_node(hf, node_from);
    node_t*         target_node = read_node(hf, node_to);

    if (source_node->first_relationship == UNINITIALIZED_LONG
        || target_node->first_relationship == UNINITIALIZED_LONG) {
        return NULL;
    }

    unsigned long next_id  = source_node->first_relationship;
    unsigned long start_id = next_id;

    do {
        rel = read_relationship(hf, next_id);
        if ((direction != INCOMING && rel->source_node == node_from
             && rel->target_node == node_to)
            || (direction != OUTGOING && rel->source_node == node_to
                && rel->target_node == node_from)) {
            return rel;
        }
        next_id = next_relationship_id(hf, source_node->id, rel, direction);
    } while (next_id != start_id && next_id != UNINITIALIZED_LONG);

    return NULL;
}
