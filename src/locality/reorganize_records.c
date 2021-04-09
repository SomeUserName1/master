#include "reorganize_records.h"

#include "../access/in_memory_file.h"
#include "../constants.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/htable.h"
#include "../data-struct/list_node.h"
#include "../data-struct/list_rel.h"
#include "../data-struct/list_ul.h"
#include "../data-struct/set_ul.h"
#include "../record/node.h"
#include "../record/relationship.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRESS_SHARE (10000)

int
ul_cmp(const void* a, const void* b)
{
    unsigned long ai = *(unsigned long*)a;
    unsigned long bi = *(unsigned long*)b;

    return ai < bi ? -1 : ai == bi ? 0 : 1;
}

unsigned long*
remap_node_ids(in_memory_file_t* db, unsigned long* partition)
{
    if (!db || !partition || db->node_id_counter < 1) {
        printf("remap node ids: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long progress_counter = 0;

    set_ul_t* part_numbers = create_set_ul();
    // Count the number of partitions
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }

        set_ul_insert(part_numbers, partition[i]);
    }

    unsigned long* partition_nums =
          calloc(set_ul_size(part_numbers), sizeof(unsigned long));
    htable_iterator_t* it    = create_htable_iterator((htable_t*)part_numbers);
    size_t             i     = 0;
    void*              key   = NULL;
    void*              value = NULL;
    while (htable_iterator_next(it, &key, &value) == 0) {
        partition_nums[i] = *(unsigned long*)key;
        ++i;
    }

    qsort(partition_nums,
          set_ul_size(part_numbers),
          sizeof(unsigned long),
          ul_cmp);

    dict_ul_ul_t* part_to_idx = create_dict_ul_ul();
    for (size_t i = 0; i < set_ul_size(part_numbers); ++i) {
        dict_ul_ul_insert(part_to_idx, partition_nums[i], i);
    }
    free(partition_nums);
    printf("Collected number of partitions.\n");

    // Construct per partition a list of nodes
    list_ul_t** nodes_per_partition =
          calloc(set_ul_size(part_numbers), sizeof(list_ul_t*));

    for (size_t i = 0; i < set_ul_size(part_numbers); ++i) {
        nodes_per_partition[i] = create_list_ul();
    }

    size_t idx       = 0;
    progress_counter = 0;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }
        idx = dict_ul_ul_get_direct(part_to_idx, partition[i]);
        list_ul_append(nodes_per_partition[idx], i);
    }
    printf("Created nodes per partition lists.\n");

    // assign id as position in the list + length of all lists up to now
    unsigned long* new_node_ids =
          calloc(db->node_id_counter, sizeof(unsigned long));

    list_node_t*  nodes = in_memory_get_nodes(db);
    node_t*       current;
    unsigned long id_counter = 0;

    for (size_t i = 0; i < set_ul_size(part_numbers); ++i) {
        printf("Processed %lu partitions\n", i);
        for (size_t j = 0; j < list_ul_size(nodes_per_partition[i]); ++j) {
            current =
                  list_node_get(nodes, list_ul_get(nodes_per_partition[i], j));
            new_node_ids[current->id] = id_counter;
            id_counter++;
        }
        list_ul_destroy(nodes_per_partition[i]);
    }
    free(nodes_per_partition);
    set_ul_destroy(part_numbers);

    progress_counter = 0;
    // apply the new record ids and insert them into a new dict
    dict_ul_node_t* new_node_cache = create_dict_ul_node();
    for (size_t i = 0; i < list_node_size(nodes); ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }

        current     = list_node_get(nodes, i);
        current->id = new_node_ids[current->id];
        dict_ul_node_insert(new_node_cache, current->id, node_copy(current));
    }
    list_node_destroy(nodes);
    dict_ul_node_destroy(db->cache_nodes);
    db->cache_nodes = new_node_cache;

    printf("Applied new node ID mapping to nodes.\n");

    progress_counter = 0;
    // iterate over all relationships updating the ids of the nodes
    list_relationship_t* rels = in_memory_get_relationships(db);
    relationship_t*      rel;
    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }
        rel              = list_relationship_get(rels, i);
        rel->source_node = new_node_ids[rel->source_node];
        rel->target_node = new_node_ids[rel->target_node];
    }

    printf("Applied new node ID mapping to relationships.\n");

    list_relationship_destroy(rels);
    return new_node_ids;
}

unsigned long*
remap_rel_ids(in_memory_file_t* db)
{
    unsigned long* rel_ids = calloc(db->rel_id_counter, sizeof(unsigned long));
    unsigned long  progress_counter = 0;
    // for each node, fetch the outgoing set and assign them new ids, based on
    // their nodes.
    list_relationship_t* rels;
    list_node_t*         nodes      = in_memory_get_nodes(db);
    unsigned long        id_counter = 0;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }
        rels = in_memory_expand(db, i, OUTGOING);
        for (size_t j = 0; j < list_relationship_size(rels); ++j) {
            rel_ids[list_relationship_get(rels, j)->id] = id_counter;
            id_counter++;
        }
        list_relationship_destroy(rels);
    }

    printf("Build new edge ID mapping.\n");

    // Apply new IDs to rels
    rels = in_memory_get_relationships(db);
    relationship_t* rel;
    dict_ul_rel_t*  new_cache_rels = create_dict_ul_rel();
    progress_counter               = 0;
    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }
        rel                  = list_relationship_get(rels, i);
        rel->id              = rel_ids[rel->id];
        rel->prev_rel_source = rel_ids[rel->prev_rel_source];
        rel->prev_rel_target = rel_ids[rel->prev_rel_target];
        rel->next_rel_source = rel_ids[rel->next_rel_source];
        rel->next_rel_target = rel_ids[rel->next_rel_target];
        dict_ul_rel_insert(new_cache_rels, rel->id, relationship_copy(rel));
    }
    list_relationship_destroy(rels);
    dict_ul_rel_destroy(db->cache_rels);
    db->cache_rels = new_cache_rels;

    printf("Applied new edge ID mapping to edges.\n");

    // Apply new ids to nodes first relationship pointers
    node_t* node;
    progress_counter = 0;
    for (size_t i = 0; i < list_node_size(nodes); ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }
        node = list_node_get(nodes, i);
        if (node->first_relationship != UNINITIALIZED_LONG) {
            node->first_relationship = rel_ids[node->first_relationship];
        }
    }

    printf("Applied new edge ID mapping to nodes.\n");

    list_node_destroy(nodes);
    return rel_ids;
}

void
sort_incidence_list(in_memory_file_t* db)
{

    list_node_t*         nodes = in_memory_get_nodes(db);
    unsigned long        node_id;
    list_relationship_t* rels;
    set_ul_t*            rel_ids;
    unsigned long*       sorted_rel_ids;
    relationship_t*      rel;
    size_t               rels_size;
    unsigned long        progress_counter = 0;

    // For each node get the incident edges.
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        if (i % PROGRESS_SHARE == 0) {
            progress_counter++;
            printf("Processed %lu\n", progress_counter * PROGRESS_SHARE);
        }
        node_id   = list_node_get(nodes, i)->id;
        rels      = in_memory_expand(db, node_id, BOTH);
        rels_size = list_relationship_size(rels);

        if (rels_size < 3) {
            list_relationship_destroy(rels);
            continue;
        }

        // Sort the incident edges by id
        rel_ids = create_set_ul();

        for (size_t j = 0; j < rels_size; ++j) {
            rel = list_relationship_get(rels, j);

            set_ul_insert(rel_ids, rel->id);
        }

        sorted_rel_ids = calloc(set_ul_size(rel_ids), sizeof(unsigned long));

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
        list_relationship_destroy(rels);
        set_ul_destroy(rel_ids);
        free(sorted_rel_ids);
    }
    list_node_destroy(nodes);
}

void
reorganize_records(in_memory_file_t* db, unsigned long* graph_partition)
{
    // remap the node ids, update the source and target node ids in the
    // relationships
    free(remap_node_ids(db, graph_partition));

    // for each node find the set of outgoing edges and assign the
    // relationship id accordingly
    free(remap_rel_ids(db));
}
