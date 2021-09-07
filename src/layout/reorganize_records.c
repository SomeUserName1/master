#include "layout/reorganize_records.h"

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/set.h"

#include <stdio.h>
#include <stdlib.h>

void
prepare_move_node(heap_file* hf, unsigned long id, unsigned long to_id)
{
    if (!hf || id == UNINITIALIZED_LONG) {
        printf("heap file - move node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = read_node_internal(hf, id, true);

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
prepare_move_relationship(heap_file* hf, unsigned long id, unsigned long to_id)
{
    if (!hf || id == UNINITIALIZED_LONG) {
        printf("heap file - move relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    // Go through both incidence lists and check if the relationship to be
    // moved appears there. if so, adjust the id
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
        node = read_node_internal(hf, rel->source_node, true);
        node->first_relationship = to_id;
        update_node_internal(hf, node, true);
    }

    if ((rel->flags & FIRST_REL_TARGET_FLAG) != 0) {
        node = read_node_internal(hf, rel->target_node, true);
        node->first_relationship = to_id;
        update_node(hf, node);
    }
}

void
swap_record_page(heap_file* hf, size_t fst, size_t snd, file_type ft)
{
    if (!hf || fst >= MAX_PAGE_NO || snd > MAX_PAGE_NO) {
        printf("heap file - swap_pages: Invalid Arguments\n");
        exit(EXIT_FAILURE);
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

    page* fst_page   = pin_page(hf->cache, fst, records, ft);
    page* snd_page   = pin_page(hf->cache, snd, records, ft);
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

    unsigned char* buf = malloc(sizeof(unsigned char) * PAGE_SIZE);
    memcpy(buf, fst_page->data, PAGE_SIZE);
    memcpy(fst_page->data, snd_page->data, PAGE_SIZE);
    memcpy(snd_page->data, buf, PAGE_SIZE);

#ifdef VERBOSE
    char* type = ft == node_ft ? "node" : "rel";
    fprintf(hf->log_file,
            "swap_%s_pages %lu\nSwap_%s_pages %lu\n",
            type,
            fst,
            type,
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

    unpin_page(hf->cache, header_id_fst, header, ft);
    unpin_page(hf->cache, header_id_snd, header, ft);
    unpin_page(hf->cache, snd, records, ft);
    unpin_page(hf->cache, fst, records, ft);

    free(buf);
    free(fst_header);
    free(snd_header);
}

unsigned long*
remap_node_ids(heap_file* hf, const unsigned long* partition)
{
    if (!hf || !partition || hf->n_nodes < 1) {
        printf("remap node ids: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    set_ul* part_numbers = s_ul_create();
    // Count the number of partitions
    for (size_t i = 0; i < hf->n_nodes; ++i) {
        set_ul_insert(part_numbers, partition[i]);
    }

    unsigned long* partition_nums =
          calloc(set_ul_size(part_numbers), sizeof(unsigned long));
    set_ul_iterator* it   = set_ul_iterator_create(part_numbers);
    size_t           i    = 0;
    unsigned long    elem = UNINITIALIZED_LONG;
    while (set_ul_iterator_next(it, &elem) == 0) {
        partition_nums[i] = elem;
        ++i;
    }
    set_ul_iterator_destroy(it);

    qsort(partition_nums,
          set_ul_size(part_numbers),
          sizeof(unsigned long),
          ul_cmp);

    dict_ul_ul* part_to_idx = d_ul_ul_create();
    for (size_t i = 0; i < set_ul_size(part_numbers); ++i) {
        dict_ul_ul_insert(part_to_idx, partition_nums[i], i);
    }
    free(partition_nums);
    printf("Collected number of partitions.\n");

    // Construct per partition a list of nodes
    array_list_ul** nodes_per_partition =
          calloc(set_ul_size(part_numbers), sizeof(array_list_ul*));

    for (size_t i = 0; i < set_ul_size(part_numbers); ++i) {
        nodes_per_partition[i] = al_ul_create();
    }

    size_t idx;
    for (size_t i = 0; i < hf->n_nodes; ++i) {
        idx = dict_ul_ul_get_direct(part_to_idx, partition[i]);
        array_list_ul_append(nodes_per_partition[idx], i);
    }
    dict_ul_ul_destroy(part_to_idx);
    printf("Created nodes per partition lists.\n");

    // assign id as position in the list + length of all lists up to now
    unsigned long* new_node_ids = calloc(hf->n_nodes, sizeof(unsigned long));

    array_list_node* nodes = get_nodes(hf);
    node_t*          current;
    unsigned long    id_counter = 0;

    for (size_t i = 0; i < set_ul_size(part_numbers); ++i) {
        printf("Processed %lu partitions\n", i);
        for (size_t j = 0; j < array_list_ul_size(nodes_per_partition[i]);
             ++j) {
            current = array_list_node_get(
                  nodes, array_list_ul_get(nodes_per_partition[i], j));
            new_node_ids[current->id] = id_counter;
            id_counter++;
        }
        array_list_ul_destroy(nodes_per_partition[i]);
    }
    free(nodes_per_partition);
    set_ul_destroy(part_numbers);

    dict_ul_node* new_node_cache = d_ul_node_create();
    for (size_t i = 0; i < array_list_node_size(nodes); ++i) {
        current     = array_list_node_get(nodes, i);
        current->id = new_node_ids[current->id];
        dict_ul_node_insert(new_node_cache, current->id, node_copy(current));
    }
    array_list_node_destroy(nodes);

    printf("Applied new node ID mapping to nodes.\n");

    // iterate over all relationships updating the ids of the nodes
    array_list_relationship* rels = get_relationships(hf);
    relationship_t*          rel;
    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        rel              = array_list_relationship_get(rels, i);
        rel->source_node = new_node_ids[rel->source_node];
        rel->target_node = new_node_ids[rel->target_node];
    }

    printf("Applied new node ID mapping to relationships.\n");

    array_list_relationship_destroy(rels);
    return new_node_ids;
}

unsigned long*
remap_rel_ids(in_memory_file_t* db)
{
    unsigned long* rel_ids = calloc(db->rel_id_counter, sizeof(unsigned long));
    // for each node, fetch the outgoing set and assign them new ids, based on
    // their nodes.
    array_list_relationship* rels;
    array_list_node*         nodes      = in_memory_get_nodes(db);
    unsigned long            id_counter = 0;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, OUTGOING);
        for (size_t j = 0; j < array_list_relationship_size(rels); ++j) {
            rel_ids[array_list_relationship_get(rels, j)->id] = id_counter;
            id_counter++;
        }
        array_list_relationship_destroy(rels);
    }

    printf("Build new edge ID mapping.\n");

    // Apply new IDs to rels
    rels = in_memory_get_relationships(db);
    relationship_t* rel;
    dict_ul_rel*    new_cache_rels = d_ul_rel_create();
    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        rel                  = array_list_relationship_get(rels, i);
        rel->id              = rel_ids[rel->id];
        rel->prev_rel_source = rel_ids[rel->prev_rel_source];
        rel->prev_rel_target = rel_ids[rel->prev_rel_target];
        rel->next_rel_source = rel_ids[rel->next_rel_source];
        rel->next_rel_target = rel_ids[rel->next_rel_target];
        dict_ul_rel_insert(new_cache_rels, rel->id, relationship_copy(rel));
    }
    array_list_relationship_destroy(rels);
    dict_ul_rel_destroy(db->cache_rels);
    db->cache_rels = new_cache_rels;

    printf("Applied new edge ID mapping to edges.\n");

    // Apply new ids to nodes first relationship pointers
    node_t* node;
    for (size_t i = 0; i < array_list_node_size(nodes); ++i) {
        node = array_list_node_get(nodes, i);
        if (node->first_relationship != UNINITIALIZED_LONG) {
            node->first_relationship = rel_ids[node->first_relationship];
        }
    }

    printf("Applied new edge ID mapping to nodes.\n");

    array_list_node_destroy(nodes);
    return rel_ids;
}

void
sort_incidence_list(in_memory_file_t* db)
{

    array_list_node*         nodes = in_memory_get_nodes(db);
    unsigned long            node_id;
    array_list_relationship* rels;
    set_ul*                  rel_ids;
    unsigned long*           sorted_rel_ids;
    relationship_t*          rel;
    size_t                   rels_size;

    // For each node get the incident edges.
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        node_id   = array_list_node_get(nodes, i)->id;
        rels      = in_memory_expand(db, node_id, BOTH);
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
        rel = in_memory_get_relationship(db, sorted_rel_ids[0]);

        if (rel->source_node == node_id) {
            rel->prev_rel_source = sorted_rel_ids[rels_size - 1];
            rel->next_rel_source = sorted_rel_ids[1];
        } else {
            rel->prev_rel_target = sorted_rel_ids[rels_size - 1];
            rel->next_rel_target = sorted_rel_ids[1];
        }

        for (size_t j = 1; j < rels_size - 1; ++j) {
            rel = in_memory_get_relationship(db, sorted_rel_ids[j]);

            if (rel->source_node == node_id) {
                rel->prev_rel_source = sorted_rel_ids[j - 1];
                rel->next_rel_source = sorted_rel_ids[j + 1];
            } else {
                rel->prev_rel_target = sorted_rel_ids[j - 1];
                rel->next_rel_target = sorted_rel_ids[j + 1];
            }
        }

        rel = in_memory_get_relationship(db, sorted_rel_ids[rels_size - 1]);

        if (rel->source_node == node_id) {
            rel->prev_rel_source = sorted_rel_ids[rels_size - 2];
            rel->next_rel_source = sorted_rel_ids[0];
        } else {
            rel->prev_rel_target = sorted_rel_ids[rels_size - 2];
            rel->next_rel_target = sorted_rel_ids[0];
        }
        array_list_relationship_destroy(rels);
        set_ul_destroy(rel_ids);
        free(sorted_rel_ids);
    }
    array_list_node_destroy(nodes);
}

