#include "layout/reorganize_nodes.h"

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/set.h"
#include "query/in_memory_operators.h"

#include <stdio.h>
#include <stdlib.h>

unsigned long*
remap_node_ids(in_memory_file_t* db, const unsigned long* partition)
{
    if (!db || !partition || db->node_id_counter < 1) {
        printf("remap node ids: Invalid Arguments!\n");
        exit(-1);
    }

    set_ul* part_numbers = s_ul_create();
    // Count the number of partitions
    for (size_t i = 0; i < db->node_id_counter; ++i) {
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

    size_t idx = 0;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        idx = dict_ul_ul_get_direct(part_to_idx, partition[i]);
        array_list_ul_append(nodes_per_partition[idx], i);
    }
    dict_ul_ul_destroy(part_to_idx);
    printf("Created nodes per partition lists.\n");

    // assign id as position in the list + length of all lists up to now
    unsigned long* new_node_ids =
          calloc(db->node_id_counter, sizeof(unsigned long));

    array_list_node* nodes = in_memory_get_nodes(db);
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
    dict_ul_node_destroy(db->cache_nodes);
    db->cache_nodes = new_node_cache;

    printf("Applied new node ID mapping to nodes.\n");

    // iterate over all relationships updating the ids of the nodes
    array_list_relationship* rels = in_memory_get_relationships(db);
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

