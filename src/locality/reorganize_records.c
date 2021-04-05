#include "reorganize_records.h"

#include "../constants.h"
#include "../data-struct/list_ul.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void
remap_node_ids(in_memory_file_t* db, unsigned long* partition)
{
    if (!db || !partition || !partition[0]) {
        printf("remap node ids: Invalid Arguments!\n");
        exit(-1);
    }

    list_ul_t* part_numbers = create_list_ul();
    list_ul_append(part_numbers, partition[0]);
    // Count the number of partitions
    for (size_t i = 1; i < db->node_id_counter; ++i) {
        for (size_t j = 0; j < i; ++j) {
            if (list_ul_get(part_numbers, j) > partition[i]) {
                list_ul_insert(part_numbers, partition[i], j);
            }
        }
    }

    // Construct per partition a list of nodes
    list_ul_t** nodes_per_partition =
          calloc(list_ul_size(part_numbers), sizeof(list_ul_t*));
    for (size_t i = 0; i < list_ul_size(part_numbers); ++i) {
        printf("Part_number: %lu\n", list_ul_get(part_numbers, i));
        nodes_per_partition[i] = create_list_ul();
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        list_ul_append(nodes_per_partition[partition[i]], i);
    }

    unsigned long id_counter = 0;
    // assign id as position in the list + length of all lists up to now
    for (size_t i = 0; i < list_ul_size(part_numbers); ++i) {
        for (size_t j = 0; j < list_ul_size(nodes_per_partition[i]); ++j) {
            printf("Part: %lu, \t old id %lu, \t new id %lu\n",
                   j,
                   list_ul_get(nodes_per_partition[i], j),
                   id_counter);
            partition[list_ul_get(nodes_per_partition[i], j)] = id_counter;
            id_counter++;
        }
        list_ul_destroy(nodes_per_partition[i]);
    }
    free(nodes_per_partition);
    free(part_numbers);

    // apply the new record ids
    list_node_t* nodes = in_memory_get_nodes(db);
    node_t*      current;
    for (size_t i = 0; i < list_node_size(nodes); ++i) {
        current     = list_node_get(nodes, i);
        current->id = partition[current->id];
    }
    list_node_destroy(nodes);

    // iterate over all relationships updating the ids of the nodes
    list_relationship_t* rels = in_memory_get_relationships(db);
    relationship_t*      rel;
    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel              = list_relationship_get(rels, i);
        rel->source_node = partition[rel->source_node];
        rel->target_node = partition[rel->target_node];
    }
    list_relationship_destroy(rels);
}

void
remap_rel_ids(in_memory_file_t* db)
{
    unsigned long* rel_ids = calloc(db->rel_id_counter, sizeof(unsigned long));

    // for each node, fetch the outgoing set and assign them new ids, based on
    // their nodes.
    list_relationship_t* rels;
    unsigned long        id_counter = 0;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, OUTGOING);
        for (size_t j = 0; j < list_relationship_size(rels); ++j) {
            rel_ids[list_relationship_get(rels, j)->id] = id_counter;
            id_counter++;
        }
    }

    // Apply new IDs to rels
    relationship_t* rel;
    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel                  = list_relationship_get(rels, i);
        rel->id              = rel_ids[rel->id];
        rel->prev_rel_source = rel_ids[rel->prev_rel_source];
        rel->prev_rel_target = rel_ids[rel->prev_rel_target];
        rel->next_rel_source = rel_ids[rel->next_rel_source];
        rel->next_rel_target = rel_ids[rel->next_rel_target];
    }
    list_relationship_destroy(rels);

    // Apply new ids to nodes first relationship pointers
    list_node_t* nodes = in_memory_get_nodes(db);
    node_t*      node;
    for (size_t i = 0; i < list_node_size(nodes); ++i) {
        node = list_node_get(nodes, i);
        if (node->first_relationship != UNINITIALIZED_LONG) {
            node->first_relationship = rel_ids[node->first_relationship];
        }
    }
    list_node_destroy(nodes);
}

void
sort_incidence_list(in_memory_file_t* db)
{

    list_node_t*         nodes = in_memory_get_nodes(db);
    unsigned long        node_id;
    list_relationship_t* rels;
    list_ul_t*           sorted_rel_ids;
    relationship_t*      rel;
    size_t               rels_size;

    // For each node get the incident edges.
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        node_id   = list_node_get(nodes, i)->id;
        rels      = in_memory_expand(db, node_id, BOTH);
        rels_size = list_relationship_size(rels);

        if (rels_size < 3) {
            continue;
        }

        // Sort the incident edges by id
        sorted_rel_ids = create_list_ul();
        list_ul_append(sorted_rel_ids, list_relationship_get(rels, 0)->id);

        for (size_t j = 1; j < rels_size; ++j) {
            rel = list_relationship_get(rels, j);

            for (size_t k = 0; k < j; ++k) {
                if (rel->id < list_ul_get(sorted_rel_ids, k)) {
                    list_ul_insert(sorted_rel_ids, rel->id, k);
                }
            }
        }

        // relink the incidence list pointers.
        rel = list_relationship_get(rels, list_ul_get(sorted_rel_ids, 0));

        if (rel->source_node == node_id) {
            rel->prev_rel_source = list_ul_get(sorted_rel_ids, rels_size - 1);
            rel->next_rel_source = list_ul_get(sorted_rel_ids, 1);
        } else {
            rel->prev_rel_target = list_ul_get(sorted_rel_ids, rels_size - 1);
            rel->next_rel_target = list_ul_get(sorted_rel_ids, 1);
        }

        for (size_t j = 1; j < rels_size - 1; ++j) {
            rel = list_relationship_get(rels, list_ul_get(sorted_rel_ids, j));

            if (rel->source_node == node_id) {
                rel->prev_rel_source = list_ul_get(sorted_rel_ids, j - 1);
                rel->next_rel_source = list_ul_get(sorted_rel_ids, j + 1);
            } else {
                rel->prev_rel_target = list_ul_get(sorted_rel_ids, j - 1);
                rel->next_rel_target = list_ul_get(sorted_rel_ids, j + 1);
            }
        }

        rel = list_relationship_get(rels,
                                    list_ul_get(sorted_rel_ids, rels_size - 1));

        if (rel->source_node == node_id) {
            rel->prev_rel_source = list_ul_get(sorted_rel_ids, rels_size - 2);
            rel->next_rel_source = list_ul_get(sorted_rel_ids, 0);
        } else {
            rel->prev_rel_target = list_ul_get(sorted_rel_ids, rels_size - 2);
            rel->next_rel_target = list_ul_get(sorted_rel_ids, 0);
        }
        list_relationship_destroy(rels);
        list_ul_destroy(sorted_rel_ids);
    }
    list_node_destroy(nodes);
}

void
reorganize_records(in_memory_file_t* db, unsigned long* graph_partition)
{
    // remap the node ids, update the source and target node ids in the
    // relationships
    remap_node_ids(db, graph_partition);

    // for each node find the set of outgoing edges and assign the
    // relationship id accordingly
    remap_rel_ids(db);

    // Sort the incidence list pointers by the id of the rels.
    sort_incidence_list(db);
}
