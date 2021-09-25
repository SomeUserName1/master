/*
 * @(#)heap_file.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
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

heap_file*
heap_file_create(page_cache* pc, const char* log_path)
{
    if (!pc || !log_path) {
        // LCOV_EXCL_START
        printf("heap file - create: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    heap_file* hf          = malloc(sizeof(heap_file));
    hf->cache              = pc;
    hf->last_alloc_node_id = 0;
    hf->last_alloc_rel_id  = 0;
    hf->num_reads_nodes    = 0;
    hf->num_updates_nodes  = 0;
    hf->num_reads_rels     = 0;
    hf->num_update_rels    = 0;

    array_list_node* nodes = get_nodes(hf, false);
    hf->n_nodes            = array_list_node_size(nodes);
    array_list_node_destroy(nodes);

    array_list_relationship* rels = get_relationships(hf, false);
    hf->n_rels                    = array_list_relationship_size(rels);
    array_list_relationship_destroy(rels);

    FILE* log_file = fopen(log_path, "a");

    if (log_file == NULL) {
        // LCOV_EXCL_START
        printf("heap file - create: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    hf->log_file = log_file;

    return hf;
}

void
heap_file_destroy(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap_file - destroy: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    fclose(hf->log_file);

    free(hf);
}

bool
check_record_exists(heap_file* hf, unsigned long id, bool node, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - check record exists: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned char slots = node ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;
    file_type     ft    = node ? node_ft : relationship_ft;

    unsigned long record_page_id = id >> CHAR_BIT;
    unsigned char slot_in_page   = id & UCHAR_MAX;

    if (slot_in_page % SLOTS_PER_PAGE
        > (slot_in_page + (slots - 1)) % SLOTS_PER_PAGE) {
        return false;
    }

    size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

    size_t header_id = absolute_slot / (PAGE_SIZE * CHAR_BIT);

    size_t bit_offset = absolute_slot % (PAGE_SIZE * CHAR_BIT);

    page* header_page = pin_page(hf->cache, header_id, header, ft, log);

    bool result = compare_bits(header_page->data,
                               PAGE_SIZE * CHAR_BIT,
                               UCHAR_MAX,
                               bit_offset,
                               slots);

    unpin_page(hf->cache, header_id, header, ft, log);

    return result;
}

void
next_free_slots(heap_file* hf, bool node, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - next free slots: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    file_type     ft      = node ? node_ft : relationship_ft;
    unsigned long n_slots = node ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;

    unsigned long* prev_allocd_id =
          node ? &(hf->last_alloc_node_id) : &(hf->last_alloc_rel_id);

    unsigned long cur_id         = *prev_allocd_id;
    unsigned long record_page_id = *prev_allocd_id >> CHAR_BIT;
    unsigned char slot_in_page   = *prev_allocd_id & UCHAR_MAX;

    do {
        if (!check_record_exists(hf, cur_id, node, log)
            && slot_in_page % SLOTS_PER_PAGE
                     <= (slot_in_page + (n_slots - 1)) % SLOTS_PER_PAGE) {
            *prev_allocd_id = cur_id;
            return;
        }

        if ((slot_in_page + n_slots) % SLOTS_PER_PAGE
            < slot_in_page % SLOTS_PER_PAGE) {
            record_page_id++;
            slot_in_page = 0;
        } else {
            slot_in_page += n_slots;
        }

        cur_id = (record_page_id << CHAR_BIT) | slot_in_page;
    } while (record_page_id < hf->cache->pdb->records[ft]->num_pages);

    page* np = new_page(hf->cache, ft, log);

    if (np->page_no > (unsigned long)(ULONG_MAX << CHAR_BIT)) {
        // LCOV_EXCL_START
        printf("heap file - next free slot: Reached size limit of Database\n"
               "Unreachable in most ile systems as file size limit is by "
               "orders of magnitude smaller (16 TiB FS limit vs. 1.6 EiB DB "
               "limit\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    } else {
        *prev_allocd_id = np->page_no << CHAR_BIT;
        unpin_page(hf->cache, np->page_no, records, ft, log);
    }
}

static node_t*
read_node_internal(heap_file*    hf,
                   unsigned long node_id,
                   bool          check_exists,
                   bool          log)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - read node internal: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (check_exists && !check_record_exists(hf, node_id, true, log)) {
        // LCOV_EXCL_START
        printf("heap file - read node internal: Node does not exist!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long page_id = node_id >> CHAR_BIT;
    page* node_page       = pin_page(hf->cache, page_id, records, node_ft, log);

    node_t* node = new_node();
    node->id     = node_id;
    node_read(node, node_page);

    unpin_page(hf->cache, page_id, records, node_ft, log);

    hf->num_reads_nodes++;

    return node;
}

static relationship_t*
read_relationship_internal(heap_file*    hf,
                           unsigned long rel_id,
                           bool          check_exists,
                           bool          log)
{
    if (!hf || rel_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - read relationship internal: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (check_exists && !check_record_exists(hf, rel_id, false, log)) {
        // LCOV_EXCL_START
        printf("heap file - read relationship internal: Relationship does "
               "not "
               "exist!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long page_id = rel_id >> CHAR_BIT;
    page*         rel_page =
          pin_page(hf->cache, page_id, records, relationship_ft, log);

    relationship_t* rel = new_relationship();
    rel->id             = rel_id;
    relationship_read(rel, rel_page);

    unpin_page(hf->cache, page_id, records, relationship_ft, log);

    hf->num_reads_rels++;

    return rel;
}

static void
update_node_internal(heap_file* hf,
                     node_t*    node_to_write,
                     bool       check_exists,
                     bool       log)
{
    if (!hf || !node_to_write || node_to_write->id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - update node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (check_exists
        && !check_record_exists(hf, node_to_write->id, true, log)) {
        // LCOV_EXCL_START
        printf("heap file - update node internal: Node does not exist!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long page_id = node_to_write->id >> CHAR_BIT;
    page* node_page       = pin_page(hf->cache, page_id, records, node_ft, log);

    node_write(node_to_write, node_page);

    unpin_page(hf->cache, page_id, records, node_ft, log);

    hf->num_updates_nodes++;
}

static void
update_relationship_internal(heap_file*      hf,
                             relationship_t* rel_to_write,
                             bool            check_exists,
                             bool            log)
{
    if (!hf || !rel_to_write || rel_to_write->id == UNINITIALIZED_LONG
        || rel_to_write->source_node == UNINITIALIZED_LONG
        || rel_to_write->target_node == UNINITIALIZED_LONG
        || rel_to_write->weight == UNINITIALIZED_WEIGHT) {
        // LCOV_EXCL_START
        printf("heap file - update relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (check_exists
        && !check_record_exists(hf, rel_to_write->id, false, log)) {
        // LCOV_EXCL_START
        printf("heap file - update relationship internal: Relationship "
               "does not "
               "exist!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long page_id = rel_to_write->id >> CHAR_BIT;
    page*         rel_page =
          pin_page(hf->cache, page_id, records, relationship_ft, log);

    relationship_write(rel_to_write, rel_page);

    unpin_page(hf->cache, page_id, records, relationship_ft, log);

    hf->num_update_rels++;
}

unsigned long
create_node(heap_file* hf, unsigned long label, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - create node: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    next_free_slots(hf, true, log);
    unsigned long node_id = hf->last_alloc_node_id;

    node_t* node = new_node();
    node->id     = node_id;
    node->label  = label;

    update_node_internal(hf, node, false, log);

    if (log) {
        fprintf(hf->log_file, "Create_Node %lu %lu\n", node_id, node->label);
        fflush(hf->log_file);
    }

    free(node);

    unsigned long record_page_id = node_id >> CHAR_BIT;
    unsigned char slot_in_page   = node_id & UCHAR_MAX;

    size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

    size_t header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
    size_t byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
    size_t bit_offset  = absolute_slot % CHAR_BIT;

    page* header_page = pin_page(hf->cache, header_id, header, node_ft, log);

    unsigned char* used_bits = malloc(sizeof(unsigned char));
    used_bits[0]             = UCHAR_MAX;

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_NODE,
               used_bits,
               log);

    unpin_page(hf->cache, header_id, header, node_ft, log);

    hf->n_nodes++;

    return node_id;
}

unsigned long
create_relationship(heap_file*    hf,
                    unsigned long from_node_id,
                    unsigned long to_node_id,
                    double        weight,
                    unsigned long label,
                    bool          log)
{
    if (!hf || from_node_id == UNINITIALIZED_LONG
        || to_node_id == UNINITIALIZED_LONG || weight == UNINITIALIZED_WEIGHT) {
        // LCOV_EXCL_START
        printf("heap file - create relationship: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    next_free_slots(hf, false, log);
    unsigned long rel_id = hf->last_alloc_rel_id;

    relationship_t* rel = new_relationship();
    rel->id             = rel_id;
    rel->source_node    = from_node_id;
    rel->target_node    = to_node_id;
    rel->weight         = weight;
    rel->label          = label;

    node_t* from_node = read_node(hf, from_node_id, log);
    node_t* to_node   = read_node(hf, to_node_id, log);

    relationship_t* first_rel_from;
    relationship_t* last_rel_from;
    relationship_t* first_rel_to;
    relationship_t* last_rel_to;
    unsigned long   temp_id;

    // Find first and last relationship in source node's chain
    if (from_node->first_relationship == UNINITIALIZED_LONG) {
        last_rel_from  = rel;
        first_rel_from = rel;
    } else {
        first_rel_from =
              read_relationship(hf, from_node->first_relationship, log);

        temp_id = from_node->id == first_rel_from->source_node
                        ? first_rel_from->prev_rel_source
                        : first_rel_from->prev_rel_target;

        last_rel_from = temp_id == from_node->first_relationship
                              ? first_rel_from
                              : read_relationship(hf, temp_id, log);
    }

    // Find first and last relationship in target node's chain
    if (to_node->first_relationship == UNINITIALIZED_LONG) {
        last_rel_to  = rel;
        first_rel_to = rel;
    } else {
        if (to_node->first_relationship == first_rel_from->id) {
            first_rel_to = first_rel_from;
        } else if (to_node->first_relationship == last_rel_from->id) {
            first_rel_to = last_rel_from;
        } else {
            first_rel_to =
                  read_relationship(hf, to_node->first_relationship, log);
        }

        temp_id = to_node->id == first_rel_to->source_node
                        ? first_rel_to->prev_rel_source
                        : first_rel_to->prev_rel_target;

        if (temp_id == first_rel_from->id) {
            last_rel_to = first_rel_from;
        } else if (temp_id == last_rel_from->id) {
            last_rel_to = last_rel_from;
        } else if (temp_id == to_node->first_relationship) {
            last_rel_to = first_rel_to;
        } else {
            last_rel_to = read_relationship(hf, temp_id, log);
        }
    }

    rel->prev_rel_source = last_rel_from->id;
    rel->next_rel_source = first_rel_from->id;
    rel->prev_rel_target = last_rel_to->id;
    rel->next_rel_target = first_rel_to->id;

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
    }
    if (last_rel_to->target_node == to_node_id) {
        last_rel_to->next_rel_target = rel->id;
    }

    // Adjust previous pointer in source node's next relation
    if (first_rel_from->source_node == from_node_id) {
        first_rel_from->prev_rel_source = rel->id;
    }
    if (first_rel_from->target_node == from_node_id) {
        first_rel_from->prev_rel_target = rel->id;
    }
    // Adjust previous pointer in target node's next relation
    if (first_rel_to->source_node == to_node_id) {
        first_rel_to->prev_rel_source = rel->id;
    }
    if (first_rel_to->target_node == to_node_id) {
        first_rel_to->prev_rel_target = rel->id;
    }

    // Set the first relationship pointer, if the inserted rel is the first
    // rel
    if (from_node->first_relationship == UNINITIALIZED_LONG) {
        from_node->first_relationship = rel->id;
        update_node(hf, from_node, log);
    }
    free(from_node);

    if (to_node->first_relationship == UNINITIALIZED_LONG) {
        to_node->first_relationship = rel->id;
        update_node(hf, to_node, log);
    }
    free(to_node);

    // in case one of the previous and next pointers of source and target are to
    // the same relationship, we must only update and free them once!
    if (first_rel_from != rel && first_rel_from != last_rel_from
        && first_rel_from != first_rel_to && first_rel_from != last_rel_to) {
        update_relationship_internal(hf, first_rel_from, false, log);
        free(first_rel_from);
    }

    if (last_rel_from != rel && last_rel_from != first_rel_to
        && last_rel_from != last_rel_to) {
        update_relationship_internal(hf, last_rel_from, false, log);
        free(last_rel_from);
    }

    if (first_rel_to != rel && first_rel_to != last_rel_to) {
        update_relationship_internal(hf, first_rel_to, false, log);
        free(first_rel_to);
    }
    if (last_rel_to != rel) {
        update_relationship_internal(hf, last_rel_to, false, log);
        free(last_rel_to);
    }

    update_relationship_internal(hf, rel, false, log);

    if (log) {
        fprintf(hf->log_file, "create_rel %lu %lu\n", rel->id, rel->label);
        fflush(hf->log_file);
    }

    free(rel);

    unsigned long record_page_id = rel_id >> CHAR_BIT;
    unsigned char slot_in_page   = rel_id & UCHAR_MAX;

    size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

    size_t header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
    size_t byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
    size_t bit_offset  = absolute_slot % CHAR_BIT;

    unsigned char* used_bits = malloc(sizeof(unsigned char));
    used_bits[0]             = UCHAR_MAX;

    page* header_page =
          pin_page(hf->cache, header_id, header, relationship_ft, log);

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_REL,
               used_bits,
               log);

    unpin_page(hf->cache, header_id, header, relationship_ft, log);

    hf->n_rels++;

    return rel_id;
}

node_t*
read_node(heap_file* hf, unsigned long node_id, bool log)
{
    node_t* node = read_node_internal(hf, node_id, true, log);

    if (log) {
        fprintf(hf->log_file, "Read_Node %lu %lu\n", node_id, node->label);
        fflush(hf->log_file);
    }

    return node;
}

relationship_t*
read_relationship(heap_file* hf, unsigned long rel_id, bool log)
{
    relationship_t* rel = read_relationship_internal(hf, rel_id, true, log);

    if (log) {
        fprintf(hf->log_file, "read_rel %lu %lu\n", rel->id, rel->label);
        fflush(hf->log_file);
    }

    return rel;
}

void
update_node(heap_file* hf, node_t* node_to_write, bool log)
{
    update_node_internal(hf, node_to_write, true, log);

    if (log) {
        fprintf(hf->log_file,
                "update_node %lu %lu\n",
                node_to_write->id,
                node_to_write->label);
        fflush(hf->log_file);
    }
}

void
update_relationship(heap_file* hf, relationship_t* rel_to_write, bool log)
{
    update_relationship_internal(hf, rel_to_write, true, log);

    if (log) {
        fprintf(hf->log_file,
                "update_rel %lu %lu\n",
                rel_to_write->id,
                rel_to_write->label);
        fflush(hf->log_file);
    }
}

void
delete_node(heap_file* hf, unsigned long node_id, bool log)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - delete node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    node_t* node = read_node_internal(hf, node_id, true, log);

    if (node->first_relationship != UNINITIALIZED_LONG) {
        relationship_t* rel =
              read_relationship(hf, node->first_relationship, log);
        unsigned long prev_rel_id = node_id == rel->source_node
                                          ? rel->prev_rel_source
                                          : rel->prev_rel_target;

        while (prev_rel_id != UNINITIALIZED_LONG) {
            rel         = read_relationship(hf, prev_rel_id, log);
            prev_rel_id = node_id == rel->source_node ? rel->prev_rel_source
                                                      : rel->prev_rel_target;
            delete_relationship(hf, rel->id, log);
        }
    }

    if (log) {
        fprintf(hf->log_file, "delete_node %lu %lu\n", node_id, node->label);
        fflush(hf->log_file);
    }
    free(node);

    unsigned long record_page_id = node_id >> CHAR_BIT;
    unsigned char slot_in_page   = node_id & UCHAR_MAX;

    size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

    size_t         header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
    size_t         byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
    size_t         bit_offset  = absolute_slot % CHAR_BIT;
    unsigned char* unused_bits = malloc(sizeof(unsigned char));
    unused_bits[0]             = 0;
    page* header_page = pin_page(hf->cache, header_id, header, node_ft, log);

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_NODE,
               unused_bits,
               log);

    unpin_page(hf->cache, header_id, header, node_ft, log);

    if (node_id < hf->last_alloc_node_id) {
        hf->last_alloc_node_id = node_id;
    }

    hf->n_nodes--;
}

void
delete_relationship(heap_file* hf, unsigned long rel_id, bool log)
{
    if (!hf || rel_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - delete relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    relationship_t* rel = read_relationship(hf, rel_id, log);

    if (rel_id != rel->prev_rel_source) {
        relationship_t* prev_rel_from =
              read_relationship(hf, rel->prev_rel_source, log);

        relationship_t* next_rel_from =
              rel->next_rel_target == prev_rel_from->id
                    ? prev_rel_from
                    : read_relationship(hf, rel->next_rel_source, log);

        relationship_t* prev_rel_to =
              rel->prev_rel_target == prev_rel_from->id ? prev_rel_from
              : rel->prev_rel_target == next_rel_from->id
                    ? next_rel_from
                    : read_relationship(hf, rel->prev_rel_target, log);

        relationship_t* next_rel_to =
              rel->next_rel_target == prev_rel_from->id   ? prev_rel_from
              : rel->next_rel_target == next_rel_from->id ? next_rel_from
              : rel->next_rel_target == prev_rel_to->id
                    ? prev_rel_to
                    : read_relationship(hf, rel->next_rel_target, log);

        // Adjust next pointer in source node's previous relation
        if (prev_rel_from->source_node == rel->source_node) {
            prev_rel_from->next_rel_source = rel->next_rel_source;
        }
        if (prev_rel_from->target_node == rel->source_node) {
            prev_rel_from->next_rel_target = rel->next_rel_source;
        }

        // Adjust previous pointer in source node's next relation
        if (next_rel_from->source_node == rel->source_node) {
            next_rel_from->prev_rel_source = rel->prev_rel_source;
        }
        if (next_rel_from->target_node == rel->source_node) {
            next_rel_from->prev_rel_target = rel->prev_rel_source;
        }

        // Adjust next pointer in target node's previous relation
        if (prev_rel_to->source_node == rel->target_node) {
            prev_rel_to->next_rel_source = rel->next_rel_target;
        }
        if (prev_rel_to->target_node == rel->target_node) {
            prev_rel_to->next_rel_target = rel->next_rel_target;
        }

        // Adjust previous pointer in target node's next relation
        if (next_rel_to->source_node == rel->target_node) {
            next_rel_to->prev_rel_source = rel->prev_rel_target;
        }
        if (next_rel_to->target_node == rel->target_node) {
            next_rel_to->prev_rel_target = rel->prev_rel_target;
        }

        // in case one of the previous and next pointers of source and target
        // are to the same relationship, we must only update and free them once!
        if (next_rel_from != rel && next_rel_from != prev_rel_from
            && next_rel_from != next_rel_to && next_rel_from != prev_rel_to) {
            update_relationship_internal(hf, next_rel_from, false, log);
            free(next_rel_from);
        }

        if (prev_rel_from != rel && prev_rel_from != next_rel_to
            && prev_rel_from != prev_rel_to) {
            update_relationship_internal(hf, prev_rel_from, false, log);
            free(prev_rel_from);
        }

        if (next_rel_to != rel && next_rel_to != prev_rel_to) {
            update_relationship_internal(hf, next_rel_to, false, log);
            free(next_rel_to);
        }
        if (prev_rel_to != rel) {
            update_relationship_internal(hf, prev_rel_to, false, log);
            free(prev_rel_to);
        }
    }

    node_t* node = read_node_internal(hf, rel->source_node, true, log);

    if (node->first_relationship == rel->id) {
        if (rel->next_rel_source == rel->id) {
            node->first_relationship = UNINITIALIZED_LONG;
        } else {
            node->first_relationship = rel->next_rel_source;
        }

        update_node(hf, node, log);
    }
    free(node);

    node = read_node_internal(hf, rel->target_node, true, log);
    if (node->first_relationship == rel->id) {
        if (rel->next_rel_target == rel->id) {
            node->first_relationship = UNINITIALIZED_LONG;
        } else {
            node->first_relationship = rel->next_rel_target;
        }

        update_node(hf, node, log);
    }
    free(node);

    if (log) {
        fprintf(hf->log_file, "delete_rel %lu %lu\n", rel_id, rel->label);
        fflush(hf->log_file);
    }

    free(rel);

    unsigned long record_page_id = rel_id >> CHAR_BIT;
    unsigned char slot_in_page   = rel_id & UCHAR_MAX;

    size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

    size_t         header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
    size_t         byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
    size_t         bit_offset  = absolute_slot % CHAR_BIT;
    unsigned char* unused_bits = malloc(sizeof(unsigned char));
    unused_bits[0]             = 0;
    page* header_page =
          pin_page(hf->cache, header_id, header, relationship_ft, log);

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_REL,
               unused_bits,
               log);

    unpin_page(hf->cache, header_id, header, relationship_ft, log);

    if (rel_id < hf->last_alloc_rel_id) {
        hf->last_alloc_rel_id = rel_id;
    }

    hf->n_rels--;
}

array_list_node*
get_nodes(heap_file* hf, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - get nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_node* result = al_node_create();
    node_t*          node;
    unsigned long    cur_id         = 0;
    unsigned long    record_page_id = 0;
    unsigned char    slot_in_page   = 0;

    do {
        if (check_record_exists(hf, cur_id, true, log)) {
            node = read_node_internal(hf, cur_id, false, log);

            if (log) {
                fprintf(hf->log_file,
                        "read_node %lu %lu\n",
                        node->id,
                        node->label);
                fflush(hf->log_file);
            }

            array_list_node_append(result, node);
        }

        if ((slot_in_page + NUM_SLOTS_PER_NODE) % SLOTS_PER_PAGE
            < slot_in_page % SLOTS_PER_PAGE) {
            record_page_id++;
            slot_in_page = 0;
        } else {
            slot_in_page += NUM_SLOTS_PER_NODE;
        }

        cur_id = (record_page_id << CHAR_BIT) | slot_in_page;
    } while (record_page_id < hf->cache->pdb->records[node_ft]->num_pages);

    return result;
}

array_list_relationship*
get_relationships(heap_file* hf, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - get relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_relationship* result = al_rel_create();
    relationship_t*          rel;

    unsigned long cur_id         = 0;
    unsigned long record_page_id = 0;
    unsigned char slot_in_page   = 0;

    do {
        if (check_record_exists(hf, cur_id, false, log)) {
            rel = read_relationship_internal(hf, cur_id, false, log);

            if (log) {
                fprintf(
                      hf->log_file, "read_rel %lu %lu\n", rel->id, rel->label);
                fflush(hf->log_file);
            }

            array_list_relationship_append(result, rel);
        }

        if ((slot_in_page + NUM_SLOTS_PER_REL) % SLOTS_PER_PAGE
            < slot_in_page % SLOTS_PER_PAGE) {
            record_page_id++;
            slot_in_page = 0;
        } else {
            slot_in_page += NUM_SLOTS_PER_REL;
        }

        cur_id = (record_page_id << CHAR_BIT) | slot_in_page;
    } while (record_page_id
             < hf->cache->pdb->records[relationship_ft]->num_pages);

    return result;
}

unsigned long
next_relationship_id(heap_file*      hf,
                     unsigned long   node_id,
                     relationship_t* rel,
                     direction_t     direction,
                     bool            log)
{
    if (!hf || node_id == UNINITIALIZED_LONG || !rel) {
        // LCOV_EXCL_START
        printf("heap_file - next relationship id: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long start_rel_id = rel->id;
    unsigned long rel_id = node_id == rel->source_node ? rel->next_rel_source
                                                       : rel->next_rel_target;

    do {
        rel = read_relationship(hf, rel_id, log);

        if (rel_id != start_rel_id
            && ((rel->source_node == node_id && direction != INCOMING)
                || (rel->target_node == node_id && direction != OUTGOING))) {
            rel_id = rel->id;
            free(rel);
            return rel_id;
        }
        rel_id = node_id == rel->source_node ? rel->next_rel_source
                                             : rel->next_rel_target;
        free(rel);

    } while (rel_id != start_rel_id);

    return UNINITIALIZED_LONG;
}

array_list_relationship*
expand(heap_file* hf, unsigned long node_id, direction_t direction, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - expand: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    node_t* node = read_node(hf, node_id, log);

    array_list_relationship* result = al_rel_create();
    unsigned long            rel_id = node->first_relationship;
    free(node);

    if (rel_id == UNINITIALIZED_LONG) {
        return result;
    }

    relationship_t* rel = read_relationship(hf, rel_id, log);
    unsigned long   start_id;

    if ((rel->source_node == node_id && direction != INCOMING)
        || (rel->target_node == node_id && direction != OUTGOING)) {
        start_id = rel_id;
    } else {
        rel_id   = next_relationship_id(hf, node_id, rel, direction, log);
        start_id = rel_id;
    }
    free(rel);

    while (rel_id != UNINITIALIZED_LONG) {
        rel = read_relationship(hf, rel_id, log);
        array_list_relationship_append(result, rel);
        rel_id = next_relationship_id(hf, node_id, rel, direction, log);

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
                              direction_t   direction,
                              bool          log)
{
    if (!hf || direction > BOTH) {
        // LCOV_EXCL_START
        printf("heap file - contains relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (node_from == UNINITIALIZED_LONG || node_to == UNINITIALIZED_LONG
        || !check_record_exists(hf, node_from, true, log)
        || !check_record_exists(hf, node_to, true, log)) {
        return NULL;
    }

    relationship_t* rel;
    node_t*         source_node = read_node_internal(hf, node_from, false, log);
    node_t*         target_node = read_node_internal(hf, node_to, false, log);

    if (log) {
        fprintf(hf->log_file,
                "read_node %lu %lu\n",
                source_node->id,
                source_node->label);
        fprintf(hf->log_file,
                "read_node %lu %lu\n",
                target_node->id,
                target_node->label);
        fflush(hf->log_file);
    }

    if (source_node->first_relationship == UNINITIALIZED_LONG
        || target_node->first_relationship == UNINITIALIZED_LONG) {
        free(source_node);
        free(target_node);
        return NULL;
    }

    unsigned long next_id  = source_node->first_relationship;
    unsigned long start_id = next_id;

    free(source_node);
    free(target_node);

    do {
        rel = read_relationship(hf, next_id, log);
        if ((direction != INCOMING && rel->source_node == node_from
             && rel->target_node == node_to)
            || (direction != OUTGOING && rel->source_node == node_to
                && rel->target_node == node_from)) {
            return rel;
        }

        next_id = next_relationship_id(hf, node_from, rel, direction, log);

        free(rel);
    } while (next_id != start_id && next_id != UNINITIALIZED_LONG);

    return NULL;
}

node_t*
find_node(heap_file* hf, unsigned long label, bool log)
{
    if (!hf || !label) {
        // LCOV_EXCL_START
        printf("heap files - find node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    node_t*       node;
    unsigned long cur_id         = 0;
    unsigned long record_page_id = 0;
    unsigned char slot_in_page   = 0;

    do {
        if (check_record_exists(hf, cur_id, true, log)) {
            node = read_node_internal(hf, cur_id, false, log);

            if (log) {
                fprintf(hf->log_file,
                        "read_node %lu %lu\n",
                        node->id,
                        node->label);
            }

            if (label == node->label) {
                return node;
            }
            free(node);
        }

        if ((slot_in_page + NUM_SLOTS_PER_NODE) % SLOTS_PER_PAGE
            < slot_in_page % SLOTS_PER_PAGE) {
            record_page_id++;
            slot_in_page = 0;
        } else {
            slot_in_page += NUM_SLOTS_PER_NODE;
        }

        cur_id = (record_page_id << CHAR_BIT) | slot_in_page;
    } while (record_page_id < hf->cache->pdb->records[node_ft]->num_pages);

    // LCOV_EXCL_START
    printf("heap file - find node: No node with label %lu in database!\n",
           label);
    exit(EXIT_FAILURE);
    // LCOV_EXCL_STOP
}

relationship_t*
find_relationships(heap_file* hf, unsigned long label, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("heap file - get relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    relationship_t* rel;
    unsigned long   cur_id         = 0;
    unsigned long   record_page_id = 0;
    unsigned char   slot_in_page   = 0;

    do {
        if (check_record_exists(hf, cur_id, false, log)) {
            rel = read_relationship_internal(hf, cur_id, false, log);

            if (log) {
                fprintf(
                      hf->log_file, "read_rel %lu %lu\n", rel->id, rel->label);
                fflush(hf->log_file);
            }

            if (label == rel->label) {
                return rel;
            }
            free(rel);
        }

        if ((slot_in_page + NUM_SLOTS_PER_REL) % SLOTS_PER_PAGE
            < slot_in_page % SLOTS_PER_PAGE) {
            record_page_id++;
            slot_in_page = 0;
        } else {
            slot_in_page += NUM_SLOTS_PER_REL;
        }

        cur_id = (record_page_id << CHAR_BIT) | slot_in_page;
    } while (record_page_id
             < hf->cache->pdb->records[relationship_ft]->num_pages);

    // LCOV_EXCL_START
    printf("heap file - find relationship: No relationship with label %lu in "
           "database!\n",
           label);
    exit(EXIT_FAILURE);
    // LCOV_EXCL_STOP}
}
