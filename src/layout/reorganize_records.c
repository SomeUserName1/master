/*
 * @(#)reorganize_records.c  1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "layout/reorganize_records.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/header_page.h"
#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/set.h"
#include "disk_file.h"
#include "page_cache.h"
#include "physical_database.h"

void
swap_nodes(heap_file* hf, unsigned long fst, unsigned long snd, bool log)
{
    // The node ids must be within the current amount pages and they must not be
    // on a page boundary
    if (!hf || fst == UNINITIALIZED_LONG || snd == UNINITIALIZED_LONG
        || (fst >> CHAR_BIT) >= hf->cache->pdb->records[node_ft]->num_pages
        || (snd >> CHAR_BIT) >= hf->cache->pdb->records[node_ft]->num_pages
        || (fst % SLOTS_PER_PAGE)
                 > ((fst + NUM_SLOTS_PER_NODE - 1) % SLOTS_PER_PAGE)
        || (snd % SLOTS_PER_PAGE)
                 > ((snd + NUM_SLOTS_PER_NODE - 1) % SLOTS_PER_PAGE)) {
        // LCOV_EXCL_START
        printf("reorder records - swao node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    bool fst_exists = check_record_exists(hf, fst, true, log);
    bool snd_exists = check_record_exists(hf, snd, true, log);

    if (fst == snd || (!fst_exists && !snd_exists)) {
        return;
    }

    relationship_t*          rel;
    array_list_relationship* rels;
    if (fst_exists) {
        rels = expand(hf, fst, BOTH, log);
    } else {
        rels = al_rel_create();
    }

    if (snd_exists) {
        array_list_relationship* rels_snd = expand(hf, snd, BOTH, log);

        for (size_t i = 0; i < array_list_relationship_size(rels_snd); ++i) {
            rel = array_list_relationship_get(rels_snd, i);
            if (!array_list_relationship_contains(rels, rel)) {
                array_list_relationship_append(rels, rel);
            }
        }

        for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
            rel = array_list_relationship_get(rels, i);

            if (rel->source_node == fst) {
                rel->source_node = snd;
            } else if (rel->source_node == snd) {
                rel->source_node = fst;
            }

            if (rel->target_node == fst) {
                rel->target_node = snd;
            } else if (rel->target_node == snd) {
                rel->target_node = fst;
            }

            update_relationship(hf, rel, log);
        }
        array_list_relationship_destroy(rels);
    }

    node_t* fst_node;
    node_t* snd_node;

    if (fst_exists) {
        fst_node = read_node(hf, fst, log);
    }

    if (snd_exists) {
        snd_node = read_node(hf, snd, log);
    }

    if (log) {
        fprintf(hf->log_file,
                "swap_nodes %lu %lu %lu %lu\n",
                fst,
                fst_exists ? fst_node->label : UNINITIALIZED_LONG,
                snd,
                snd_exists ? snd_node->label : UNINITIALIZED_LONG);
    }

    if (fst_exists != snd_exists) {
        unsigned long record_page_id = fst >> CHAR_BIT;
        unsigned char slot_in_page   = fst & UCHAR_MAX;

        size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

        size_t header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
        size_t byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
        size_t bit_offset  = absolute_slot % CHAR_BIT;

        unsigned char* used_bits = malloc(sizeof(unsigned char));
        used_bits[0]             = snd_exists ? UCHAR_MAX : 0;

        page* header_page =
              pin_page(hf->cache, header_id, header, node_ft, log);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_NODE,
                   used_bits,
                   log);

        unpin_page(hf->cache, header_id, header, node_ft, log);
        record_page_id = snd >> CHAR_BIT;
        slot_in_page   = snd & UCHAR_MAX;

        absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

        header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
        byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
        bit_offset  = absolute_slot % CHAR_BIT;

        used_bits    = malloc(sizeof(unsigned char));
        used_bits[0] = fst_exists ? UCHAR_MAX : 0;

        header_page = pin_page(hf->cache, header_id, header, node_ft, log);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_NODE,
                   used_bits,
                   log);

        unpin_page(hf->cache, header_id, header, node_ft, log);
    }

    if (fst_exists) {
        fst_node->id = snd;
        update_node(hf, fst_node, log);
    }

    if (snd_exists) {
        snd_node->id = fst;
        update_node(hf, snd_node, log);
    }
}

void
swap_relationships(heap_file*    hf,
                   unsigned long fst,
                   unsigned long snd,
                   bool          log)
{
    if (!hf || fst == UNINITIALIZED_LONG || snd == UNINITIALIZED_LONG
        || (fst >> CHAR_BIT)
                 >= hf->cache->pdb->records[relationship_ft]->num_pages
        || (snd >> CHAR_BIT)
                 >= hf->cache->pdb->records[relationship_ft]->num_pages
        || (fst % SLOTS_PER_PAGE)
                 > ((fst + NUM_SLOTS_PER_REL - 1) % SLOTS_PER_PAGE)
        || (snd % SLOTS_PER_PAGE)
                 > ((snd + NUM_SLOTS_PER_REL - 1) % SLOTS_PER_PAGE)) {
        // LCOV_EXCL_START
        printf("reorder records - swap relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    bool fst_exists = check_record_exists(hf, fst, false, log);
    bool snd_exists = check_record_exists(hf, snd, false, log);

    if (fst == snd || (!fst_exists && !snd_exists)) {
        return;
    }

    relationship_t* fst_rel = NULL;
    relationship_t* snd_rel = NULL;

    node_t*         fst_src           = NULL;
    node_t*         fst_trgt          = NULL;
    relationship_t* fst_prev_rel_from = NULL;
    relationship_t* fst_next_rel_from = NULL;
    relationship_t* fst_prev_rel_to   = NULL;
    relationship_t* fst_next_rel_to   = NULL;

    node_t*         snd_src           = NULL;
    node_t*         snd_trgt          = NULL;
    relationship_t* snd_prev_rel_from = NULL;
    relationship_t* snd_next_rel_from = NULL;
    relationship_t* snd_prev_rel_to   = NULL;
    relationship_t* snd_next_rel_to   = NULL;

    if (fst_exists) {
        fst_rel = read_relationship(hf, fst, log);
        // adjust the id in the nodes first relationship fields if neccessary
        fst_src = read_node(hf, fst_rel->source_node, log);
        if (fst_rel->id == fst_src->first_relationship) {
            fst_src->first_relationship = snd;
        }

        fst_trgt = fst_rel->target_node == fst_src->id
                         ? fst_src
                         : read_node(hf, fst_rel->target_node, log);
        if (fst_rel->id == fst_trgt->first_relationship) {
            fst_trgt->first_relationship = snd;
        }

        // Adjust next pointer in source node's previous relation
        fst_prev_rel_from =
              fst_rel && fst == fst_rel->prev_rel_source ? fst_rel
              : snd_rel && snd == fst_rel->prev_rel_source
                    ? snd_rel
                    : read_relationship(hf, fst_rel->prev_rel_source, log);

        if (fst_prev_rel_from->source_node == fst_rel->source_node) {
            fst_prev_rel_from->next_rel_source = snd;
        } else {
            fst_prev_rel_from->next_rel_target = snd;
        }

        // Adjust next pointer in target node's previous relation
        fst_prev_rel_to =
              fst_rel && fst_rel->prev_rel_target == fst   ? fst_rel
              : snd_rel && fst_rel->prev_rel_target == snd ? snd_rel
              : fst_rel->prev_rel_target == fst_prev_rel_from->id
                    ? fst_prev_rel_from
                    : read_relationship(hf, fst_rel->prev_rel_target, log);

        if (fst_prev_rel_to->source_node == fst_rel->target_node) {
            fst_prev_rel_to->next_rel_source = snd;
        } else {
            fst_prev_rel_to->next_rel_target = snd;
        }

        // Adjust previous pointer in source node's next relation
        fst_next_rel_from =
              fst_rel && fst_rel->next_rel_source == fst   ? fst_rel
              : snd_rel && fst_rel->next_rel_source == snd ? snd_rel
              : fst_rel->next_rel_source == fst_prev_rel_from->id
                    ? fst_prev_rel_from
              : fst_rel->next_rel_source == fst_prev_rel_to->id
                    ? fst_prev_rel_to
                    : read_relationship(hf, fst_rel->next_rel_source, log);

        if (fst_next_rel_from->source_node == fst_rel->source_node) {
            fst_next_rel_from->prev_rel_source = snd;
        } else {
            fst_next_rel_from->prev_rel_target = snd;
        }

        // Adjust previous pointer in target node's next relation
        fst_next_rel_to =
              fst_rel && fst_rel->next_rel_target == fst   ? fst_rel
              : snd_rel && fst_rel->next_rel_target == snd ? snd_rel
              : fst_rel->next_rel_target == fst_prev_rel_from->id
                    ? fst_prev_rel_from
              : fst_rel->next_rel_target == fst_prev_rel_to->id
                    ? fst_prev_rel_to
              : fst_rel->next_rel_target == fst_next_rel_from->id
                    ? fst_next_rel_from
                    : read_relationship(hf, fst_rel->next_rel_target, log);

        if (fst_next_rel_to->source_node == fst_rel->target_node) {
            fst_next_rel_to->prev_rel_source = snd;
        } else {
            fst_next_rel_to->prev_rel_target = snd;
        }
    }

    if (snd_exists) {
        snd_rel = read_relationship(hf, snd, log);
        // adjust the id in the nodes first relationship fields if neccessary
        snd_src = (fst_src && snd_rel->source_node == fst_src->id) ? fst_src
                  : (fst_trgt && snd_rel->source_node == fst_trgt->id)
                        ? fst_trgt
                        : read_node(hf, snd_rel->source_node, log);

        if (snd_rel->id == snd_src->first_relationship) {
            snd_src->first_relationship = fst;
        }

        snd_trgt = (fst_src && snd_rel->target_node == fst_src->id) ? fst_src
                   : (fst_trgt && snd_rel->target_node == fst_trgt->id)
                         ? fst_trgt
                   : snd_rel->target_node == snd_src->id
                         ? snd_src
                         : read_node(hf, snd_rel->target_node, log);
        if (snd_rel->id == snd_trgt->first_relationship) {
            snd_trgt->first_relationship = fst;
        }

        // Adjust next pointer in source node's previous relation
        snd_prev_rel_from =
              snd_rel->prev_rel_source == fst   ? fst_rel
              : snd_rel->prev_rel_source == snd ? snd_rel
              : (fst_prev_rel_from
                 && snd_rel->prev_rel_source == fst_prev_rel_from->id)
                    ? fst_prev_rel_from
              : (fst_prev_rel_to
                 && snd_rel->prev_rel_source == fst_prev_rel_to->id)
                    ? fst_prev_rel_to
              : (fst_next_rel_from
                 && snd_rel->prev_rel_source == fst_next_rel_from->id)
                    ? fst_next_rel_from
              : (fst_next_rel_to
                 && snd_rel->prev_rel_source == fst_next_rel_to->id)
                    ? fst_next_rel_to
                    : read_relationship(hf, snd_rel->prev_rel_source, log);

        if (snd_prev_rel_from->source_node == snd_rel->source_node) {
            snd_prev_rel_from->next_rel_source = fst;
        } else {
            snd_prev_rel_from->next_rel_target = fst;
        }

        // Adjust next pointer in target node's previous relation
        snd_prev_rel_to =
              snd_rel->prev_rel_target == fst   ? fst_rel
              : snd_rel->prev_rel_target == snd ? snd_rel
              : (fst_prev_rel_from
                 && snd_rel->prev_rel_target == fst_prev_rel_from->id)
                    ? fst_prev_rel_from
              : (fst_prev_rel_to
                 && snd_rel->prev_rel_target == fst_prev_rel_to->id)
                    ? fst_prev_rel_to
              : (fst_next_rel_from
                 && snd_rel->prev_rel_target == fst_next_rel_from->id)
                    ? fst_next_rel_from
              : (fst_next_rel_to
                 && snd_rel->prev_rel_target == fst_next_rel_to->id)
                    ? fst_next_rel_to
              : snd_rel->prev_rel_target == snd_prev_rel_from->id
                    ? snd_prev_rel_from
                    : read_relationship(hf, snd_rel->prev_rel_target, log);

        if (snd_prev_rel_to->source_node == snd_rel->target_node) {
            snd_prev_rel_to->next_rel_source = fst;
        } else {
            snd_prev_rel_to->next_rel_target = fst;
        }

        // Adjust previous pointer in source node's next relation
        snd_next_rel_from =
              snd_rel->next_rel_source == fst   ? fst_rel
              : snd_rel->next_rel_source == snd ? snd_rel
              : (fst_prev_rel_from
                 && snd_rel->next_rel_source == fst_prev_rel_from->id)
                    ? fst_prev_rel_from
              : (fst_prev_rel_to
                 && snd_rel->next_rel_source == fst_prev_rel_to->id)
                    ? fst_prev_rel_to
              : (fst_next_rel_from
                 && snd_rel->next_rel_source == fst_next_rel_from->id)
                    ? fst_next_rel_from
              : (fst_next_rel_to
                 && snd_rel->next_rel_source == fst_next_rel_to->id)
                    ? fst_next_rel_to
              : snd_rel->next_rel_source == snd_prev_rel_from->id
                    ? snd_prev_rel_from
              : snd_rel->next_rel_source == snd_prev_rel_to->id
                    ? snd_prev_rel_to
                    : read_relationship(hf, snd_rel->next_rel_source, log);

        if (snd_next_rel_from->source_node == snd_rel->source_node) {
            snd_next_rel_from->prev_rel_source = fst;
        } else {
            snd_next_rel_from->prev_rel_target = fst;
        }

        // Adjust previous pointer in target node's next relation
        snd_next_rel_to =
              snd_rel->next_rel_target == fst   ? fst_rel
              : snd_rel->next_rel_target == snd ? snd_rel
              : (fst_prev_rel_from
                 && snd_rel->next_rel_target == fst_prev_rel_from->id)
                    ? fst_prev_rel_from
              : (fst_prev_rel_to
                 && snd_rel->next_rel_target == fst_prev_rel_to->id)
                    ? fst_prev_rel_to
              : (fst_next_rel_from
                 && snd_rel->next_rel_target == fst_next_rel_from->id)
                    ? fst_next_rel_from
              : (fst_next_rel_to
                 && snd_rel->next_rel_target == fst_next_rel_to->id)
                    ? fst_next_rel_to
              : snd_rel->next_rel_target == snd_prev_rel_from->id
                    ? snd_prev_rel_from
              : snd_rel->next_rel_target == snd_prev_rel_to->id
                    ? snd_prev_rel_to
              : snd_rel->next_rel_target == snd_next_rel_from->id
                    ? snd_next_rel_from
                    : read_relationship(hf, snd_rel->next_rel_target, log);

        if (snd_next_rel_to->source_node == snd_rel->target_node) {
            snd_next_rel_to->prev_rel_source = fst;
        } else {
            snd_next_rel_to->prev_rel_target = fst;
        }

        if (snd_src != fst_src && snd_src != fst_trgt && snd_src != snd_trgt) {
            update_node(hf, snd_src, log);
            free(snd_src);
        }
        if (snd_trgt != fst_src && snd_trgt != fst_trgt) {
            update_node(hf, snd_trgt, log);
            free(snd_trgt);
        }

        // in case one of the previous and next pointers of source and target
        // are to the same relationship, we must only update and free them once!
        if (snd_next_rel_to != fst_rel && snd_next_rel_to != snd_rel
            && snd_next_rel_to != snd_next_rel_from
            && snd_next_rel_to != snd_prev_rel_to
            && snd_next_rel_to != snd_prev_rel_from
            && snd_next_rel_to != fst_next_rel_to
            && snd_next_rel_to != fst_next_rel_from
            && snd_next_rel_to != fst_prev_rel_to
            && snd_next_rel_to != fst_prev_rel_from) {
            update_relationship(hf, snd_next_rel_to, log);
            free(snd_next_rel_to);
        }

        if (snd_next_rel_from != fst_rel && snd_next_rel_from != snd_rel
            && snd_next_rel_from != snd_prev_rel_to
            && snd_next_rel_from != snd_prev_rel_from
            && snd_next_rel_from != fst_next_rel_to
            && snd_next_rel_from != fst_next_rel_from
            && snd_next_rel_from != fst_prev_rel_to
            && snd_next_rel_from != fst_prev_rel_from) {
            update_relationship(hf, snd_next_rel_from, log);
            free(snd_next_rel_from);
        }

        if (snd_prev_rel_to != fst_rel && snd_prev_rel_to != snd_rel
            && snd_prev_rel_to != snd_prev_rel_from
            && snd_prev_rel_to != fst_next_rel_to
            && snd_prev_rel_to != fst_next_rel_from
            && snd_prev_rel_to != fst_prev_rel_to
            && snd_prev_rel_to != fst_prev_rel_from) {
            update_relationship(hf, snd_prev_rel_to, log);
            free(snd_prev_rel_to);
        }

        if (snd_prev_rel_from != fst_rel && snd_prev_rel_from != snd_rel
            && snd_prev_rel_from != fst_next_rel_to
            && snd_prev_rel_from != fst_next_rel_from
            && snd_prev_rel_from != fst_prev_rel_to
            && snd_prev_rel_from != fst_prev_rel_from) {
            update_relationship(hf, snd_prev_rel_from, log);
            free(snd_prev_rel_from);
        }
    }

    if (fst_exists) {
        if (fst_src != fst_trgt) {
            update_node(hf, fst_src, log);
            free(fst_src);
        }

        update_node(hf, fst_trgt, log);
        free(fst_trgt);

        // in case one of the previous and next pointers of source and target
        // are to the same relationship, we must only update and free them once!
        if (fst_next_rel_to != fst_rel && fst_next_rel_to != snd_rel
            && fst_next_rel_to != fst_next_rel_from
            && fst_next_rel_to != fst_prev_rel_to
            && fst_next_rel_to != fst_prev_rel_from) {
            update_relationship(hf, fst_next_rel_to, log);
            free(fst_next_rel_to);
        }

        if (fst_next_rel_from != fst_rel && fst_next_rel_from != snd_rel
            && fst_next_rel_from != fst_prev_rel_to
            && fst_next_rel_from != fst_prev_rel_from) {
            update_relationship(hf, fst_next_rel_from, log);
            free(fst_next_rel_from);
        }

        if (fst_prev_rel_to != fst_rel && fst_prev_rel_to != snd_rel
            && fst_prev_rel_to != fst_prev_rel_from) {
            update_relationship(hf, fst_prev_rel_to, log);
            free(fst_prev_rel_to);
        }

        if (fst_prev_rel_from != fst_rel && fst_prev_rel_from != snd_rel) {
            update_relationship(hf, fst_prev_rel_from, log);
            free(fst_prev_rel_from);
        }
    }

    if (log) {
        fprintf(hf->log_file, "swap_relationships %lu %lu\n", fst, snd);
    }

    if (fst_exists != snd_exists) {
        unsigned long record_page_id = fst >> CHAR_BIT;
        unsigned char slot_in_page   = fst & UCHAR_MAX;

        size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

        size_t header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
        size_t byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
        size_t bit_offset  = absolute_slot % CHAR_BIT;

        unsigned char* used_bits = malloc(sizeof(unsigned char));
        used_bits[0]             = snd_exists ? UCHAR_MAX : 0;

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

        record_page_id = snd >> CHAR_BIT;
        slot_in_page   = snd & UCHAR_MAX;

        absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

        header_id    = absolute_slot / (PAGE_SIZE * CHAR_BIT);
        byte_offset  = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
        bit_offset   = absolute_slot % CHAR_BIT;
        used_bits    = malloc(sizeof(unsigned char));
        used_bits[0] = fst_exists ? UCHAR_MAX : 0;

        header_page =
              pin_page(hf->cache, header_id, header, relationship_ft, log);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_REL,
                   used_bits,
                   log);

        unpin_page(hf->cache, header_id, header, relationship_ft, log);
    }

    if (fst_exists) {
        fst_rel->id = snd;
        update_relationship(hf, fst_rel, log);
    }

    if (snd_exists) {
        snd_rel->id = fst;
        update_relationship(hf, snd_rel, log);
    }
}

void
swap_record_pages(heap_file* hf, size_t fst, size_t snd, file_type ft, bool log)
{
    if (!hf || fst >= MAX_PAGE_NO || snd > MAX_PAGE_NO) {
        // LCOV_EXCL_START
        printf("heap file - swap_pages: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (fst == snd) {
        return;
    }

    unsigned long n_slots_records =
          ft == node_ft ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;
    unsigned long fst_id;
    unsigned long snd_id;

    for (size_t i = 0; i < SLOTS_PER_PAGE; i += n_slots_records) {
        fst_id = (fst << CHAR_BIT) | i;
        snd_id = (snd << CHAR_BIT) | i;
        if (ft == node_ft) {
            swap_nodes(hf, fst_id, snd_id, log);
        } else {
            swap_relationships(hf, fst_id, snd_id, log);
        }
    }

    if (log) {
        char* type = ft == node_ft ? "node" : "rel";
        fprintf(hf->log_file, "swap_%s_pages %lu %lu\n", type, fst, snd);
    }
}

void
reorder_nodes(heap_file* hf, dict_ul_ul* new_ids, bool log)
{
    if (!hf || !new_ids) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    // construct inverse mapping to avoid seeking for values num nodes times
    dict_ul_ul_iterator* it              = dict_ul_ul_iterator_create(new_ids);
    dict_ul_ul*          inverse_new_ids = d_ul_ul_create();
    unsigned long        old_id;
    unsigned long        new_id;

    while (dict_ul_ul_iterator_next(it, &old_id, &new_id) == 0) {
        dict_ul_ul_insert(inverse_new_ids, new_id, old_id); // fut => cur
    }
    dict_ul_ul_iterator_destroy(it);

    size_t n_ids = hf->cache->pdb->records[node_ft]->num_pages * SLOTS_PER_PAGE;
    unsigned short cur_slot = 0;
    unsigned long  cur_page = 0;
    unsigned long  temp_new;

    for (size_t i = 0; i < n_ids; ++i) {
        new_id = cur_page << CHAR_BIT | cur_slot;

        if (dict_ul_ul_get(inverse_new_ids, new_id, &old_id) == 0) {
            swap_nodes(hf, old_id, new_id, log);

            // remove the element that was just swapped
            dict_ul_ul_remove(new_ids, old_id);
            dict_ul_ul_remove(inverse_new_ids, new_id);

            if (old_id != new_id) {
                // update the dict such that the node that was previously at
                // position new id is now at old id
                temp_new = dict_ul_ul_get_direct(new_ids, new_id);
                // remove the old entries for the node that was swapped with
                dict_ul_ul_remove(new_ids, new_id);
                dict_ul_ul_remove(inverse_new_ids, temp_new);
                dict_ul_ul_insert(new_ids, old_id, temp_new);
                dict_ul_ul_insert(inverse_new_ids, temp_new, old_id);
            }
        }

        if (cur_slot == UCHAR_MAX) {
            cur_page++;
            cur_slot = 0;
        } else {
            cur_slot++;
        }
    }

    dict_ul_ul_destroy(inverse_new_ids);
}

void
reorder_nodes_by_sequence(heap_file*           hf,
                          const unsigned long* sequence,
                          bool                 log)
{
    if (!hf || !sequence) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder nodes by sequence: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    const size_t n_pages =
          hf->n_nodes * NUM_SLOTS_PER_NODE / (PAGE_SIZE * CHAR_BIT)
          + (hf->n_rels * NUM_SLOTS_PER_NODE % (PAGE_SIZE * CHAR_BIT) != 0);

    dict_ul_ul*   new_ids  = d_ul_ul_create();
    phy_database* temp_pdb = phy_database_create("reord_temp", "temp_log");

    page_cache* temp_pc = page_cache_create(temp_pdb, n_pages, "temp_log");
    heap_file*  temp_hf = heap_file_create(temp_pc, "temp_log");

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        next_free_slots(temp_hf, true, false);
        dict_ul_ul_insert(new_ids, sequence[i], temp_hf->last_alloc_node_id);
    }

    heap_file_destroy(temp_hf);
    page_cache_destroy(temp_pc);
    phy_database_delete(temp_pdb);

    reorder_nodes(hf, new_ids, log);
}

void
reorder_relationships(heap_file* hf, dict_ul_ul* new_ids, bool log)
{
    if (!hf || !new_ids) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder relationships: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    // construct inverse mapping to avoid seeking for values num nodes times
    dict_ul_ul_iterator* it              = dict_ul_ul_iterator_create(new_ids);
    dict_ul_ul*          inverse_new_ids = d_ul_ul_create();
    unsigned long        old_id;
    unsigned long        new_id;

    while (dict_ul_ul_iterator_next(it, &old_id, &new_id) == 0) {
        dict_ul_ul_insert(inverse_new_ids, new_id, old_id); // fut => cur
    }
    dict_ul_ul_iterator_destroy(it);

    size_t n_ids =
          hf->cache->pdb->records[relationship_ft]->num_pages * SLOTS_PER_PAGE;
    unsigned short cur_slot = 0;
    unsigned long  cur_page = 0;
    unsigned long  temp_new;

    for (size_t i = 0; i < n_ids; ++i) {
        new_id = cur_page << CHAR_BIT | cur_slot;

        if (dict_ul_ul_get(inverse_new_ids, new_id, &old_id) == 0) {
            swap_relationships(hf, old_id, new_id, log);

            // remove the element that was just swapped
            dict_ul_ul_remove(new_ids, old_id);
            dict_ul_ul_remove(inverse_new_ids, new_id);

            if (old_id != new_id) {
                // update the dict such that the node that was previously at
                // position new id is now at old id
                temp_new = dict_ul_ul_get_direct(new_ids, new_id);
                // remove the old entries for the node that was swapped with
                dict_ul_ul_remove(new_ids, new_id);
                dict_ul_ul_remove(inverse_new_ids, temp_new);
                dict_ul_ul_insert(new_ids, old_id, temp_new);
                dict_ul_ul_insert(inverse_new_ids, temp_new, old_id);
            }
        }

        if (cur_slot == UCHAR_MAX) {
            cur_page++;
            cur_slot = 0;
        } else {
            cur_slot++;
        }
    }
}

void
reorder_relationships_by_nodes(heap_file* hf, bool log)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("reorganize records - reorder relationships by nodes: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    // for each node, fetch the outgoing set and assign them new ids, based on
    // their nodes.
    unsigned long* new_order = calloc(hf->n_rels, sizeof(unsigned long));
    array_list_relationship* rels;
    array_list_node*         nodes = get_nodes(hf, log);

    size_t k = 0;
    for (size_t i = 0; i < hf->n_nodes; ++i) {
        rels = expand(hf, array_list_node_get(nodes, i)->id, OUTGOING, log);
        for (size_t j = 0; j < array_list_relationship_size(rels); ++j) {
            new_order[k] = array_list_relationship_get(rels, j)->id;
            k++;
        }
        array_list_relationship_destroy(rels);
    }

    array_list_node_destroy(nodes);

    reorder_relationships_by_sequence(hf, new_order, log);
    free(new_order);
}

void
reorder_relationships_by_sequence(heap_file*           hf,
                                  const unsigned long* sequence,
                                  bool                 log)
{
    if (!hf || !sequence) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder relationships by sequence: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    const size_t n_pages =
          hf->n_rels * NUM_SLOTS_PER_REL / (PAGE_SIZE * CHAR_BIT)
          + (hf->n_rels * NUM_SLOTS_PER_REL % (PAGE_SIZE * CHAR_BIT) != 0);

    dict_ul_ul*   new_ids  = d_ul_ul_create();
    phy_database* temp_pdb = phy_database_create("reord_temp", "temp_log");
    disk_file_grow(temp_pdb->header[relationship_ft], n_pages, false);

    page_cache* temp_pc = page_cache_create(temp_pdb, n_pages, "temp_log");
    heap_file*  temp_hf = heap_file_create(temp_pc, "temp_log");

    for (size_t i = 0; i < hf->n_rels; ++i) {
        next_free_slots(temp_hf, false, false);
        dict_ul_ul_insert(new_ids, sequence[i], temp_hf->last_alloc_rel_id);
    }

    heap_file_destroy(temp_hf);
    page_cache_destroy(temp_pc);
    phy_database_delete(temp_pdb);

    reorder_relationships(hf, new_ids, log);
}

void
sort_incidence_list(heap_file* hf, bool log)
{
    if (!hf) {
        // LCVOV_EXCL_START
        printf("reorganize records - sort_incidence_list: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_node*         nodes = get_nodes(hf, log);
    unsigned long            node_id;
    array_list_relationship* rels;
    set_ul*                  rel_ids;
    unsigned long*           sorted_rel_ids;
    relationship_t*          rel;
    size_t                   rels_size;

    // For each node get the incident edges.
    for (size_t i = 0; i < hf->n_nodes; ++i) {
        node_id   = array_list_node_get(nodes, i)->id;
        rels      = expand(hf, node_id, BOTH, log);
        rels_size = array_list_relationship_size(rels);

        if (rels_size < 3) {
            array_list_relationship_destroy(rels);
            continue;
        }

        // Sort the incident edges by id
        rel_ids = s_ul_create();

        for (size_t j = 0; j < rels_size; ++j) {
            rel = array_list_relationship_get(rels, j);
            set_ul_insert(rel_ids, rel->id);
        }

        sorted_rel_ids = calloc(set_ul_size(rel_ids), sizeof(unsigned long));
        set_ul_iterator* it   = set_ul_iterator_create(rel_ids);
        size_t           i    = 0;
        unsigned long    elem = UNINITIALIZED_LONG;
        while (set_ul_iterator_next(it, &elem) == 0) {
            sorted_rel_ids[i] = elem;
            ++i;
        }
        set_ul_iterator_destroy(it);

        qsort(sorted_rel_ids,
              set_ul_size(rel_ids),
              sizeof(unsigned long),
              ul_cmp);

        // relink the incidence list pointers.
        rel = read_relationship(hf, sorted_rel_ids[0], log);

        if (rel->source_node == node_id) {
            rel->prev_rel_source = sorted_rel_ids[rels_size - 1];
            rel->next_rel_source = sorted_rel_ids[1];
        } else {
            rel->prev_rel_target = sorted_rel_ids[rels_size - 1];
            rel->next_rel_target = sorted_rel_ids[1];
        }
        update_relationship(hf, rel, log);

        for (size_t j = 1; j < rels_size - 1; ++j) {
            rel = read_relationship(hf, sorted_rel_ids[j], log);

            if (rel->source_node == node_id) {
                rel->prev_rel_source = sorted_rel_ids[j - 1];
                rel->next_rel_source = sorted_rel_ids[j + 1];
            } else {
                rel->prev_rel_target = sorted_rel_ids[j - 1];
                rel->next_rel_target = sorted_rel_ids[j + 1];
            }
            update_relationship(hf, rel, log);
        }

        rel = read_relationship(hf, sorted_rel_ids[rels_size - 1], log);

        if (rel->source_node == node_id) {
            rel->prev_rel_source = sorted_rel_ids[rels_size - 2];
            rel->next_rel_source = sorted_rel_ids[0];
        } else {
            rel->prev_rel_target = sorted_rel_ids[rels_size - 2];
            rel->next_rel_target = sorted_rel_ids[0];
        }
        update_relationship(hf, rel, log);

        array_list_relationship_destroy(rels);
        set_ul_destroy(rel_ids);
        free(sorted_rel_ids);
    }
    array_list_node_destroy(nodes);
}
