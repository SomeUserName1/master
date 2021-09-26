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
        || (fst + 1) * NUM_SLOTS_PER_NODE * SLOT_SIZE - 1
                 > hf->cache->pdb->records[node_ft]->num_pages * PAGE_SIZE
        || (snd + 1) * NUM_SLOTS_PER_NODE * SLOT_SIZE - 1
                 > hf->cache->pdb->records[node_ft]->num_pages * PAGE_SIZE
        || (fst * NUM_SLOTS_PER_NODE) % SLOTS_PER_PAGE
                 > ((fst + 1) * NUM_SLOTS_PER_NODE - 1) % SLOTS_PER_PAGE
        || (snd * NUM_SLOTS_PER_NODE) % SLOTS_PER_PAGE
                 > ((snd + 1) * NUM_SLOTS_PER_NODE - 1) % SLOTS_PER_PAGE) {
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
        unsigned char slot_in_page   = snd & UCHAR_MAX;

        size_t absolute_slot = record_page_id * SLOTS_PER_PAGE + slot_in_page;

        size_t header_id   = absolute_slot / (PAGE_SIZE * CHAR_BIT);
        size_t byte_offset = (absolute_slot / CHAR_BIT) % PAGE_SIZE;
        size_t bit_offset  = absolute_slot % CHAR_BIT;

        unsigned char* used_bits = malloc(sizeof(unsigned char));
        used_bits[0]             = fst_exists ? UCHAR_MAX : 0;

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
        used_bits[0] = snd_exists ? UCHAR_MAX : 0;

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
    if (!hf || fst == UNINITIALIZED_LONG || snd == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("reorder records - swao node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    bool fst_exists = check_record_exists(hf, fst, false, log);
    bool snd_exists = check_record_exists(hf, snd, false, log);

    if (fst == snd || (!fst_exists && !snd_exists)) {
        return;
    }

    if (fst_exists) {
        relationship_t* fst_rel = read_relationship(hf, fst, log);

        // adjust the id in the nodes first relationship fields if neccessary
        node_t* node = read_node(hf, fst_rel->source_node, log);
        if (fst_rel->id == node->first_relationship) {
            node->first_relationship = snd;
            update_node(hf, node, log);
        }
        free(node);

        node = read_node(hf, fst_rel->target_node, log);
        if (fst_rel->id == node->first_relationship) {
            node->first_relationship = snd;
            update_node(hf, node, log);
        }
        free(node);

        // Adjust next pointer in source node's previous relation
        relationship_t* fst_prev_rel_from =
              fst == fst_rel->prev_rel_source
                    ? fst_rel
                    : read_relationship(hf, fst_rel->prev_rel_source, log);

        if (fst_prev_rel_from->source_node == fst_rel->source_node) {
            fst_prev_rel_from->next_rel_source = snd;
        } else {
            fst_prev_rel_from->next_rel_target = snd;
        }

        // Adjust next pointer in target node's previous relation
        relationship_t* fst_prev_rel_to =
              fst_rel->prev_rel_target ? fst_rel
              : fst_rel->prev_rel_target == fst_prev_rel_from->id
                    ? fst_prev_rel_from
                    : read_relationship(hf, fst_rel->prev_rel_target, log);

        if (fst_prev_rel_to->source_node == fst_rel->target_node) {
            fst_prev_rel_to->next_rel_source = snd;
        } else {
            fst_prev_rel_to->next_rel_target = snd;
        }

        // Adjust previous pointer in source node's next relation
        relationship_t* fst_next_rel_from =
              fst_rel->next_rel_source == fst ? fst_rel
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
        relationship_t* fst_next_rel_to =
              fst_rel->next_rel_target == fst ? fst_rel
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

        // in case one of the previous and next pointers of source and target
        // are to the same relationship, we must only update and free them once!
        if (fst_next_rel_from != fst_prev_rel_from
            && fst_next_rel_from != fst_next_rel_to
            && fst_next_rel_from != fst_prev_rel_to) {
            update_relationship(hf, fst_next_rel_from, log);
            free(fst_next_rel_from);
        }

        if (fst_prev_rel_from != fst_next_rel_to
            && fst_prev_rel_from != fst_prev_rel_to) {
            update_relationship(hf, fst_prev_rel_from, log);
            free(fst_prev_rel_from);
        }

        if (fst_next_rel_to != fst_prev_rel_to) {
            update_relationship(hf, fst_next_rel_to, log);
            free(fst_next_rel_to);
        }

        update_relationship(hf, fst_prev_rel_to, log);
        free(fst_prev_rel_to);
    }

    if (snd_exists) {

        // FIXME continue here! Also consider that the rels may overlap! i.e.
        // merge the code with the if above somehow...
        relationship_t* snd_rel = read_relationship(hf, snd, log);
    }

    if (log) {
        fprintf(hf->log_file, "swap_relationships %lu %lu\n", fst, snd);
    }

    if (fst_exists != snd_exists) {
        unsigned long header_id =
              (fst * NUM_SLOTS_PER_REL) / (PAGE_SIZE * CHAR_BIT);
        unsigned long byte_offset =
              ((fst * NUM_SLOTS_PER_REL) / CHAR_BIT) % PAGE_SIZE;
        unsigned char  bit_offset = (fst * NUM_SLOTS_PER_REL) % CHAR_BIT;
        unsigned char* used_bits  = malloc(sizeof(unsigned char));
        used_bits[0]              = fst_exists ? UCHAR_MAX : 0;

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

        header_id    = (snd * NUM_SLOTS_PER_REL) / (PAGE_SIZE * CHAR_BIT);
        byte_offset  = ((snd * NUM_SLOTS_PER_REL) / CHAR_BIT) % PAGE_SIZE;
        bit_offset   = (snd * NUM_SLOTS_PER_REL) % CHAR_BIT;
        used_bits    = malloc(sizeof(unsigned char));
        used_bits[0] = snd_exists ? UCHAR_MAX : 0;

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

// swap pairwise
void
swap_record_pages(heap_file* hf, size_t fst, size_t snd, file_type ft, bool log)
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

    page* fst_header = pin_page(hf->cache, header_id_fst, header, ft, log);
    page* snd_header = pin_page(hf->cache, header_id_snd, header, ft, log);

    unsigned char* fst_header_bits = read_bits(hf->cache,
                                               fst_header,
                                               header_byte_offset_fst,
                                               header_bit_offset_fst,
                                               SLOTS_PER_PAGE,
                                               log);

    unsigned char* snd_header_bits = read_bits(hf->cache,
                                               snd_header,
                                               header_byte_offset_snd,
                                               header_bit_offset_snd,
                                               SLOTS_PER_PAGE,
                                               log);

    unpin_page(hf->cache, header_id_fst, header, ft, log);
    unpin_page(hf->cache, header_id_snd, header, ft, log);

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
                prepare_move_node(hf, id, to_id, log);
            } else {
                prepare_move_relationship(hf, id, to_id, log);
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
                prepare_move_node(hf, id, to_id, log);

            } else {
                prepare_move_relationship(hf, id, to_id, log);
            }
        }
    }

    page* fst_page = pin_page(hf->cache, fst, records, ft, log);
    page* snd_page = pin_page(hf->cache, snd, records, ft, log);

    unsigned char* buf = malloc(sizeof(unsigned char) * PAGE_SIZE);

    memcpy(buf, fst_page->data, PAGE_SIZE);
    memcpy(fst_page->data, snd_page->data, PAGE_SIZE);
    memcpy(snd_page->data, buf, PAGE_SIZE);

    fst_page->dirty = true;
    snd_page->dirty = true;

    unpin_page(hf->cache, snd, records, ft, log);
    unpin_page(hf->cache, fst, records, ft, log);

    if (log) {
        char* type = ft == node_ft ? "node" : "rel";
        fprintf(hf->log_file, "swap_%s_pages %lu %lu\n", type, fst, snd);
    }

    fst_header = pin_page(hf->cache, header_id_fst, header, ft, log);
    snd_header = pin_page(hf->cache, header_id_snd, header, ft, log);

    write_bits(hf->cache,
               fst_header,
               header_byte_offset_fst,
               header_bit_offset_fst,
               SLOTS_PER_PAGE,
               snd_header_bits,
               log);

    write_bits(hf->cache,
               snd_header,
               header_byte_offset_snd,
               header_bit_offset_snd,
               SLOTS_PER_PAGE,
               fst_header_bits,
               log);

    unpin_page(hf->cache, header_id_fst, header, ft, log);
    unpin_page(hf->cache, header_id_snd, header, ft, log);

    free(buf);
    free(fst_header);
    free(snd_header);
}

// from i to n
// find node that should go to slot i
// switch that node with node i
// update the from id for the node that was previously at i
void
reorder_nodes(heap_file*  hf,
              dict_ul_ul* new_ids,
              disk_file*  new_header,
              bool        log)
{
    if (!hf || !new_ids) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_node* nodes = get_nodes(hf, log);

    node_t*       node;
    unsigned long new_id;

    for (size_t i = 0; hf->n_nodes; ++i) {
        node   = array_list_node_get(nodes, i);
        new_id = dict_ul_ul_get_direct(new_ids, node->id);
        prepare_move_node(hf, node->id, new_id, log);
    }

    if (new_header) {
        hf->cache->pdb->header[node_ft] = new_header;
    } else {
        disk_file* header_file      = hf->cache->pdb->header[node_ft];
        size_t     num_header_pages = header_file->num_pages;

        for (size_t i = 0; i < num_header_pages; ++i) {
            clear_page(header_file, i, log);
        }
    }

    size_t         header_id;
    size_t         byte_offset;
    unsigned char  bit_offset;
    unsigned char* used_bits;
    page*          header_page;

    for (size_t i = 0; hf->n_nodes; ++i) {
        node     = array_list_node_get(nodes, i);
        new_id   = dict_ul_ul_get_direct(new_ids, node->id);
        node->id = new_id;

        if (!new_header) {
            header_id = (new_id * NUM_SLOTS_PER_NODE) / (PAGE_SIZE * CHAR_BIT);

            byte_offset =
                  ((new_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;

            bit_offset = (new_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;

            used_bits    = malloc(sizeof(unsigned char));
            used_bits[0] = UCHAR_MAX;

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

        update_node(hf, node, log);
    }

    array_list_node_destroy(nodes);
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
    phy_database* temp_pdb = phy_database_create("reord_temp", NULL);
    disk_file_grow(temp_pdb->header[node_ft], n_pages, false);

    page_cache* temp_pc = page_cache_create(temp_pdb, n_pages, NULL);
    heap_file*  temp_hf = heap_file_create(temp_pc, NULL);

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        next_free_slots(temp_hf, true, false);
        dict_ul_ul_insert(new_ids, sequence[i], temp_hf->last_alloc_node_id);
    }

    heap_file_destroy(temp_hf);
    page_cache_destroy(temp_pc);

    reorder_nodes(hf, new_ids, temp_pdb->header[relationship_ft], log);

    char* catalogue_fname = temp_pdb->catalogue->file_name;
    disk_file_delete(temp_pdb->catalogue);
    free(catalogue_fname);

    char* header_fname;
    char* record_fname;

    header_fname = temp_pdb->header[node_ft]->file_name;
    disk_file_delete(temp_pdb->header[node_ft]);
    free(header_fname);

    for (file_type ft = 0; ft < invalid_ft; ++ft) {
        record_fname = temp_pdb->records[ft]->file_name;
        disk_file_delete(temp_pdb->records[ft]);
        free(record_fname);
    }

    free(temp_pdb);
}

// from i to n
// find rel that should go to slot i
// switch that rel with rel i
// update the from id for the rel that was previously at i
void
reorder_relationships(heap_file*  hf,
                      dict_ul_ul* new_ids,
                      disk_file*  new_header,
                      bool        log)
{
    if (!hf || !new_ids) {
        // LCOV_EXCL_START
        printf("reorder_records - reorder relationships: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_relationship* rels = get_relationships(hf, log);
    relationship_t*          rel;
    unsigned long            new_id;

    for (size_t i = 0; hf->n_rels; ++i) {
        rel    = array_list_relationship_get(rels, i);
        new_id = dict_ul_ul_get_direct(new_ids, rel->id);
        prepare_move_relationship(hf, rel->id, new_id, log);
    }

    if (new_header) {
        char* header_file_name =
              hf->cache->pdb->header[relationship_ft]->file_name;

        disk_file_delete(hf->cache->pdb->header[relationship_ft]);

        rename(new_header->file_name, header_file_name);

        hf->cache->pdb->header[relationship_ft] = new_header;
    } else {
        disk_file* header_file      = hf->cache->pdb->header[relationship_ft];
        size_t     num_header_pages = header_file->num_pages;

        for (size_t i = 0; i < num_header_pages; ++i) {
            clear_page(header_file, i, log);
        }
    }

    size_t         header_id;
    size_t         byte_offset;
    unsigned char  bit_offset;
    unsigned char* used_bits;
    page*          header_page;

    for (size_t i = 0; hf->n_rels; ++i) {
        rel     = array_list_relationship_get(rels, i);
        new_id  = dict_ul_ul_get_direct(new_ids, rel->id);
        rel->id = new_id;

        if (!new_header) {
            header_id = (new_id * NUM_SLOTS_PER_REL) / (PAGE_SIZE * CHAR_BIT);

            byte_offset = ((new_id * NUM_SLOTS_PER_REL) / CHAR_BIT) % PAGE_SIZE;

            bit_offset = (new_id * NUM_SLOTS_PER_REL) % CHAR_BIT;

            used_bits    = malloc(sizeof(unsigned char));
            used_bits[0] = UCHAR_MAX;

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

        update_relationship(hf, rel, log);
    }

    array_list_relationship_destroy(rels);
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
    unsigned long* new_order = calloc(hf->n_nodes, sizeof(unsigned long));
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
    phy_database* temp_pdb = phy_database_create("reord_temp", NULL);
    disk_file_grow(temp_pdb->header[relationship_ft], n_pages, false);

    page_cache* temp_pc = page_cache_create(temp_pdb, n_pages, NULL);
    heap_file*  temp_hf = heap_file_create(temp_pc, NULL);

    for (size_t i = 0; i < hf->n_rels; ++i) {
        next_free_slots(temp_hf, false, false);
        dict_ul_ul_insert(new_ids, sequence[i], temp_hf->last_alloc_rel_id);
    }

    heap_file_destroy(temp_hf);
    page_cache_destroy(temp_pc);

    reorder_relationships(hf, new_ids, temp_pdb->header[relationship_ft], log);

    char* catalogue_fname = temp_pdb->catalogue->file_name;
    disk_file_delete(temp_pdb->catalogue);
    free(catalogue_fname);

    char* header_fname;
    char* record_fname;

    header_fname = temp_pdb->header[node_ft]->file_name;
    disk_file_delete(temp_pdb->header[node_ft]);
    free(header_fname);

    for (file_type ft = 0; ft < invalid_ft; ++ft) {
        record_fname = temp_pdb->records[ft]->file_name;
        disk_file_delete(temp_pdb->records[ft]);
        free(record_fname);
    }

    free(temp_pdb);
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
