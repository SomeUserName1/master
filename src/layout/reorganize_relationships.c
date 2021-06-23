#include "layout/reorganize_relationships.h"

#include "../constants.h"
#include "access/node.h"
#include "access/operators.h"
#include "access/relationship.h"
#include "data-struct/array_list.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/set.h"

#include <stdio.h>
#include <stdlib.h>

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

