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
prepare_move_node(heap_file* hf, unsigned long id, unsigned long to_id)
{
    if (!hf || id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - move node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (id == to_id) {
        return;
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
        rel_id = next_relationship_id(hf, id, rel, BOTH);
    } while (rel_id != node->first_relationship);
    free(node);
}

void
prepare_move_relationship(heap_file* hf, unsigned long id, unsigned long to_id)
{
    if (!hf || id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("heap file - move relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (id == to_id) {
        return;
    }

    // Go through both incidence lists and check if the relationship to be
    // moved appears there. if so, adjust the id
    relationship_t* rel = read_relationship(hf, id);

    // Adjust next pointer in source node's previous relation
    relationship_t* prev_rel_from =
          id == rel->prev_rel_source
                ? rel
                : read_relationship(hf, rel->prev_rel_source);

    if (prev_rel_from->source_node == rel->source_node) {
        prev_rel_from->next_rel_source = to_id;
    } else {
        prev_rel_from->next_rel_target = to_id;
    }

    // Adjust next pointer in target node's previous relation
    relationship_t* prev_rel_to =
          rel->prev_rel_target ? rel
          : rel->prev_rel_target == prev_rel_from->id
                ? prev_rel_from
                : read_relationship(hf, rel->prev_rel_target);

    if (prev_rel_to->source_node == rel->target_node) {
        prev_rel_to->next_rel_source = to_id;
    } else {
        prev_rel_to->next_rel_target = to_id;
    }

    // Adjust previous pointer in source node's next relation
    relationship_t* next_rel_from =
          rel->next_rel_source == id                  ? rel
          : rel->next_rel_source == prev_rel_from->id ? prev_rel_from
          : rel->next_rel_source == prev_rel_to->id
                ? prev_rel_to
                : read_relationship(hf, rel->next_rel_source);

    if (next_rel_from->source_node == rel->source_node) {
        next_rel_from->prev_rel_source = to_id;
    } else {
        next_rel_from->prev_rel_target = to_id;
    }

    // Adjust previous pointer in target node's next relation
    relationship_t* next_rel_to =
          rel->next_rel_target == id                  ? rel
          : rel->next_rel_target == prev_rel_from->id ? prev_rel_from
          : rel->next_rel_target == prev_rel_to->id   ? prev_rel_to
          : rel->next_rel_target == next_rel_from->id
                ? next_rel_from
                : read_relationship(hf, rel->next_rel_target);

    if (next_rel_to->source_node == rel->target_node) {
        next_rel_to->prev_rel_source = to_id;
    } else {
        next_rel_to->prev_rel_target = to_id;
    }

    // in case one of the previous and next pointers of source and target are to
    // the same relationship, we must only update and free them once!
    if (next_rel_from != prev_rel_from && next_rel_from != next_rel_to
        && next_rel_from != prev_rel_to) {
        update_relationship(hf, next_rel_from);
        free(next_rel_from);
    }

    if (prev_rel_from != next_rel_to && prev_rel_from != prev_rel_to) {
        update_relationship(hf, prev_rel_from);
        free(prev_rel_from);
    }

    if (next_rel_to != prev_rel_to) {
        update_relationship(hf, next_rel_to);
        free(next_rel_to);
    }

    update_relationship(hf, prev_rel_to);
    free(prev_rel_to);

    // adjust the id in the nodes first relationship fields if neccessary
    node_t* node;
    if ((rel->flags & FIRST_REL_SOURCE_FLAG) != 0) {
        node                     = read_node(hf, rel->source_node);
        node->first_relationship = to_id;
        update_node(hf, node);
        free(node);
    }

    if ((rel->flags & FIRST_REL_TARGET_FLAG) != 0) {
        node                     = read_node(hf, rel->target_node);
        node->first_relationship = to_id;
        update_node(hf, node);
        free(node);
    }
}

void
swap_nodes(heap_file* hf, unsigned long fst, unsigned long snd)
{
    if (!hf || fst == UNINITIALIZED_LONG || snd == UNINITIALIZED_LONG) {
        printf("reorder records - swao node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    bool fst_exists = check_record_exists(hf, fst, true);
    bool snd_exists = check_record_exists(hf, snd, true);

    node_t* fst_node;
    node_t* snd_node;
    if (fst_exists) {
        prepare_move_node(hf, fst, snd);
        fst_node = read_node(hf, fst);
    }

    if (snd_exists) {
        prepare_move_node(hf, snd, fst);
        snd_node = read_node(hf, snd);
    }

#ifdef VERBOSE
    fprintf(hf->log_file, "swap_nodes %lu %lu\n", fst, snd);
#endif

    if (fst_exists != snd_exists) {
        unsigned long header_id =
              (fst * NUM_SLOTS_PER_NODE) / (PAGE_SIZE * CHAR_BIT);
        unsigned long byte_offset =
              ((fst * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;
        unsigned char  bit_offset = (fst * NUM_SLOTS_PER_NODE) % CHAR_BIT;
        unsigned char* used_bits  = malloc(sizeof(unsigned char));
        used_bits[0]              = fst_exists ? UCHAR_MAX : 0;

        page* header_page = pin_page(hf->cache, header_id, header, node_ft);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_NODE,
                   used_bits);

        unpin_page(hf->cache, header_id, header, node_ft);

        header_id    = (snd * NUM_SLOTS_PER_NODE) / (PAGE_SIZE * CHAR_BIT);
        byte_offset  = ((snd * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;
        bit_offset   = (snd * NUM_SLOTS_PER_NODE) % CHAR_BIT;
        used_bits    = malloc(sizeof(unsigned char));
        used_bits[0] = snd_exists ? UCHAR_MAX : 0;

        header_page = pin_page(hf->cache, header_id, header, node_ft);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_NODE,
                   used_bits);

        unpin_page(hf->cache, header_id, header, node_ft);
    }

    if (fst_exists) {
        fst_node->id = snd;
        update_node(hf, fst_node);
    }

    if (snd_exists) {
        snd_node->id = fst;
        update_node(hf, snd_node);
    }
}

void
swap_relationships(heap_file* hf, unsigned long fst, unsigned long snd)
{

    if (!hf || fst == UNINITIALIZED_LONG || snd == UNINITIALIZED_LONG) {
        printf("reorder records - swao node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    bool fst_exists = check_record_exists(hf, fst, false);
    bool snd_exists = check_record_exists(hf, snd, false);

    relationship_t* fst_rel;
    relationship_t* snd_rel;
    if (fst_exists) {
        prepare_move_relationship(hf, fst, snd);
        fst_rel = read_relationship(hf, fst);
    }

    if (snd_exists) {
        prepare_move_relationship(hf, snd, fst);
        snd_rel = read_relationship(hf, snd);
    }

#ifdef VERBOSE
    fprintf(hf->log_file, "swap_relationships %lu %lu\n", fst, snd);
#endif

    if (fst_exists != snd_exists) {
        unsigned long header_id =
              (fst * NUM_SLOTS_PER_REL) / (PAGE_SIZE * CHAR_BIT);
        unsigned long byte_offset =
              ((fst * NUM_SLOTS_PER_REL) / CHAR_BIT) % PAGE_SIZE;
        unsigned char  bit_offset = (fst * NUM_SLOTS_PER_REL) % CHAR_BIT;
        unsigned char* used_bits  = malloc(sizeof(unsigned char));
        used_bits[0]              = fst_exists ? UCHAR_MAX : 0;

        page* header_page =
              pin_page(hf->cache, header_id, header, relationship_ft);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_REL,
                   used_bits);

        unpin_page(hf->cache, header_id, header, relationship_ft);

        header_id    = (snd * NUM_SLOTS_PER_REL) / (PAGE_SIZE * CHAR_BIT);
        byte_offset  = ((snd * NUM_SLOTS_PER_REL) / CHAR_BIT) % PAGE_SIZE;
        bit_offset   = (snd * NUM_SLOTS_PER_REL) % CHAR_BIT;
        used_bits    = malloc(sizeof(unsigned char));
        used_bits[0] = snd_exists ? UCHAR_MAX : 0;

        header_page = pin_page(hf->cache, header_id, header, relationship_ft);

        write_bits(hf->cache,
                   header_page,
                   byte_offset,
                   bit_offset,
                   NUM_SLOTS_PER_REL,
                   used_bits);

        unpin_page(hf->cache, header_id, header, relationship_ft);
    }

    if (fst_exists) {
        fst_rel->id = snd;
        update_relationship(hf, fst_rel);
    }

    if (snd_exists) {
        snd_rel->id = fst;
        update_relationship(hf, snd_rel);
    }
}

void
swap_record_pages(heap_file* hf, size_t fst, size_t snd, file_type ft)
{
    if (!hf || fst >= MAX_PAGE_NO || snd > MAX_PAGE_NO) {
        // LCOV_EXCL_START
        printf("heap file - swap_pages: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t num_header_bits_fst = fst * SLOTS_PER_PAGE;

    size_t header_id_fst = (num_header_bits_fst / CHAR_BIT) / PAGE_SIZE;

    unsigned short header_byte_offset_fst =
          (num_header_bits_fst / CHAR_BIT) % PAGE_SIZE;

    unsigned char header_bit_offset_fst = num_header_bits_fst % CHAR_BIT;

    size_t num_header_bits_snd = snd * SLOTS_PER_PAGE;

    size_t header_id_snd = (num_header_bits_snd / CHAR_BIT) / PAGE_SIZE;

    unsigned short header_byte_offset_snd =
          (num_header_bits_snd / CHAR_BIT) % PAGE_SIZE;

    unsigned char header_bit_offset_snd = num_header_bits_snd % CHAR_BIT;

    page* fst_header = pin_page(hf->cache, header_id_fst, header, ft);
    page* snd_header = pin_page(hf->cache, header_id_snd, header, ft);

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

    unpin_page(hf->cache, header_id_fst, header, ft);
    unpin_page(hf->cache, header_id_snd, header, ft);

    unsigned char n_slots =
          ft == node_ft ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;

    unsigned char slot_used_mask = UCHAR_MAX >> (CHAR_BIT - n_slots);

    unsigned long id;
    unsigned long to_id;
    for (size_t i = 0; i < SLOTS_PER_PAGE; i += n_slots) {
        if (compare_bits(fst_header_bits,
                         SLOTS_PER_PAGE,
                         slot_used_mask,
                         i,
                         n_slots)) {
            id    = fst * SLOTS_PER_PAGE + i;
            to_id = snd * SLOTS_PER_PAGE + i;

            if (ft == node_ft) {
                prepare_move_node(hf, id, to_id);
            } else {
                prepare_move_relationship(hf, id, to_id);
            }
        }
    }

    for (size_t i = 0; i < SLOTS_PER_PAGE; i += n_slots) {
        if (compare_bits(snd_header_bits,
                         SLOTS_PER_PAGE,
                         slot_used_mask,
                         i,
                         n_slots)) {
            id    = snd * SLOTS_PER_PAGE + i;
            to_id = fst * SLOTS_PER_PAGE + i;

            if (ft == node_ft) {
                prepare_move_node(hf, id, to_id);

            } else {
                prepare_move_relationship(hf, id, to_id);
            }
        }
    }

    page* fst_page = pin_page(hf->cache, fst, records, ft);
    page* snd_page = pin_page(hf->cache, snd, records, ft);

    unsigned char* buf = malloc(sizeof(unsigned char) * PAGE_SIZE);

    memcpy(buf, fst_page->data, PAGE_SIZE);
    memcpy(fst_page->data, snd_page->data, PAGE_SIZE);
    memcpy(snd_page->data, buf, PAGE_SIZE);

    fst_page->dirty = true;
    snd_page->dirty = true;

    unpin_page(hf->cache, snd, records, ft);
    unpin_page(hf->cache, fst, records, ft);

#ifdef VERBOSE
    char* type = ft == node_ft ? "node" : "rel";
    fprintf(hf->log_file,
            "swap_%s_pages %lu\nSwap_%s_pages %lu\n",
            type,
            fst,
            type,
            snd);
#endif

    fst_header = pin_page(hf->cache, header_id_fst, header, ft);
    snd_header = pin_page(hf->cache, header_id_snd, header, ft);

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

    unpin_page(hf->cache, header_id_fst, header, ft);
    unpin_page(hf->cache, header_id_snd, header, ft);

    free(buf);
    free(fst_header);
    free(snd_header);
}

void
reorder_nodes(heap_file* hf, dict_ul_ul* new_ids)
{
    if (!hf || !new_ids) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_node* nodes = get_nodes(hf);
    node_t*          node;
    unsigned long    new_id;

    for (size_t i = 0; hf->n_nodes; ++i) {
        node   = array_list_node_get(nodes, i);
        new_id = dict_ul_ul_get_direct(new_ids, node->id);
        prepare_move_node(hf, node->id, new_id);
        node->id = new_id;
        update_node(hf, node);
    }

    array_list_node_destroy(nodes);
}

void
reorder_relationships_by_ids(heap_file* hf, dict_ul_ul* new_ids)
{
    if (!hf || !new_ids) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_relationship* rels = get_relationships(hf);
    relationship_t*          rel;
    unsigned long            new_id;

    for (size_t i = 0; hf->n_rels; ++i) {
        rel    = array_list_relationship_get(rels, i);
        new_id = dict_ul_ul_get_direct(new_ids, rel->id);
        prepare_move_relationship(hf, rel->id, new_id);
        rel->id = new_id;
        update_relationship(hf, rel);
    }

    array_list_relationship_destroy(rels);
}

void
reorder_relationships_by_nodes(heap_file* hf)
{
    if (!hf) {
        printf("reorganize records - reorder relationships by nodes: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
    }

    const size_t n_pages =
          hf->n_rels * NUM_SLOTS_PER_REL / (PAGE_SIZE * CHAR_BIT)
          + (hf->n_rels * NUM_SLOTS_PER_REL % (PAGE_SIZE * CHAR_BIT) != 0);

    dict_ul_ul*   new_ids  = d_ul_ul_create();
    phy_database* temp_pdb = phy_database_create("reord_temp");
    disk_file_grow(temp_pdb->header[relationship_ft], n_pages);

    page_cache* temp_pc = page_cache_create(temp_pdb, n_pages);
    heap_file*  temp_hf = heap_file_create(temp_pc);

    // for each node, fetch the outgoing set and assign them new ids, based on
    // their nodes.
    array_list_relationship* rels;
    array_list_node*         nodes = get_nodes(hf);

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        rels = expand(hf, array_list_node_get(nodes, i)->id, OUTGOING);
        for (size_t j = 0; j < array_list_relationship_size(rels); ++j) {
            next_free_slots(temp_hf, false);
            dict_ul_ul_insert(new_ids,
                              array_list_relationship_get(rels, j)->id,
                              temp_hf->last_alloc_rel_id);
        }
        array_list_relationship_destroy(rels);
    }

    array_list_node_destroy(nodes);
    heap_file_destroy(temp_hf);
    page_cache_destroy(temp_pc);
    phy_database_delete(temp_pdb);

    reorder_relationships_by_ids(hf, new_ids);
}

void
sort_incidence_list(heap_file* hf)
{
    if (!hf) {
        printf("reorganize records - sort_incidence_list: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
    }

    array_list_node*         nodes = get_nodes(hf);
    unsigned long            node_id;
    array_list_relationship* rels;
    set_ul*                  rel_ids;
    unsigned long*           sorted_rel_ids;
    relationship_t*          rel;
    size_t                   rels_size;

    // For each node get the incident edges.
    for (size_t i = 0; i < hf->n_nodes; ++i) {
        node_id   = array_list_node_get(nodes, i)->id;
        rels      = expand(hf, node_id, BOTH);
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
        rel = read_relationship(hf, sorted_rel_ids[0]);

        if (rel->source_node == node_id) {
            rel->prev_rel_source = sorted_rel_ids[rels_size - 1];
            rel->next_rel_source = sorted_rel_ids[1];
        } else {
            rel->prev_rel_target = sorted_rel_ids[rels_size - 1];
            rel->next_rel_target = sorted_rel_ids[1];
        }
        update_relationship(hf, rel);

        for (size_t j = 1; j < rels_size - 1; ++j) {
            rel = read_relationship(hf, sorted_rel_ids[j]);

            if (rel->source_node == node_id) {
                rel->prev_rel_source = sorted_rel_ids[j - 1];
                rel->next_rel_source = sorted_rel_ids[j + 1];
            } else {
                rel->prev_rel_target = sorted_rel_ids[j - 1];
                rel->next_rel_target = sorted_rel_ids[j + 1];
            }
            update_relationship(hf, rel);
        }

        rel = read_relationship(hf, sorted_rel_ids[rels_size - 1]);

        if (rel->source_node == node_id) {
            rel->prev_rel_source = sorted_rel_ids[rels_size - 2];
            rel->next_rel_source = sorted_rel_ids[0];
        } else {
            rel->prev_rel_target = sorted_rel_ids[rels_size - 2];
            rel->next_rel_target = sorted_rel_ids[0];
        }
        update_relationship(hf, rel);

        array_list_relationship_destroy(rels);
        set_ul_destroy(rel_ids);
        free(sorted_rel_ids);
    }
    array_list_node_destroy(nodes);
}
