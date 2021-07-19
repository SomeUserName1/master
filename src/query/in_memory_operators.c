#include "query/in_memory_operators.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"

unsigned long
in_memory_next_relationship_id(in_memory_file_t* db,
                               unsigned long     node_id,
                               relationship_t*   rel,
                               direction_t       direction)
{
    if (db == NULL || rel == NULL || node_id == UNINITIALIZED_LONG) {
        printf("in_memory - next_relationship: Arguments must be not NULL!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long start_rel_id = rel->id;
    unsigned long rel_id = node_id == rel->source_node ? rel->next_rel_source
                                                       : rel->next_rel_target;

    do {
        rel = dict_ul_rel_get_direct(db->cache_rels, rel_id);

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
in_memory_expand(in_memory_file_t* db,
                 unsigned long     node_id,
                 direction_t       direction)
{
    if (!db) {
        printf("in_memory - expand: Arguments must be not NULL!\n");
        exit(EXIT_FAILURE);
    }

    printf("node_id %lu\n", node_id);
    node_t* node = dict_ul_node_get_direct(db->cache_nodes, node_id);

    if (!node || !db) {
        printf("in_memory - expand: Arguments must be not NULL!\n");
        exit(EXIT_FAILURE);
    }

    array_list_relationship* result = al_rel_create();
    unsigned long            rel_id = node->first_relationship;

    if (rel_id == UNINITIALIZED_LONG) {
        return result;
    }

    printf("rel id start %lu\n", rel_id);
    relationship_t* rel = in_memory_get_relationship(db, rel_id);
    unsigned long   start_id;

    if ((rel->source_node == node_id && direction != INCOMING)
        || (rel->target_node == node_id && direction != OUTGOING)) {
        start_id = rel_id;
    } else {
        rel_id = in_memory_next_relationship_id(db, node_id, rel, direction);
        printf("rel id mid %lu\n", rel_id);
        start_id = rel_id;
    }

    while (rel_id != UNINITIALIZED_LONG) {
        rel = dict_ul_rel_get_direct(db->cache_rels, rel_id);
        array_list_relationship_append(result, rel);
        rel_id = in_memory_next_relationship_id(db, node->id, rel, direction);
        printf("rel id last %lu\n", rel_id);

        if (rel_id == start_id) {
            return result;
        }
    }

    return result;
}

relationship_t*
in_memory_contains_relationship_from_to(in_memory_file_t* db,
                                        unsigned long     node_from,
                                        unsigned long     node_to,
                                        direction_t       direction)
{
    if (!db) {
        printf("in_memory - contains relationship: Invalid Arguments!\n");
        return NULL;
    }

    if (node_from == UNINITIALIZED_LONG || node_to == UNINITIALIZED_LONG) {
        return NULL;
    }

    relationship_t* rel;
    node_t* source_node = dict_ul_node_get_direct(db->cache_nodes, node_from);
    node_t* target_node = dict_ul_node_get_direct(db->cache_nodes, node_to);

    if (!source_node || !target_node) {
        printf("in_memory - contains relationship: No such nodes!\n");
        exit(EXIT_FAILURE);
    }

    if (source_node->first_relationship == UNINITIALIZED_LONG
        || target_node->first_relationship == UNINITIALIZED_LONG) {
        return NULL;
    }

    node_t* traversed_node =
          source_node->degree < target_node->degree ? source_node : target_node;

    direction_t trav_dir = traversed_node->id == node_to && direction != BOTH
                                 ? direction == OUTGOING ? INCOMING : OUTGOING
                                 : direction;

    unsigned long next_id  = traversed_node->first_relationship;
    unsigned long start_id = next_id;

    do {
        rel = dict_ul_rel_get_direct(db->cache_rels, next_id);
        if ((direction != INCOMING && rel->source_node == node_from
             && rel->target_node == node_to)
            || (direction != OUTGOING && rel->source_node == node_to
                && rel->target_node == node_from)) {
            return rel;
        }
        next_id = in_memory_next_relationship_id(
              db, traversed_node->id, rel, trav_dir);
    } while (next_id != start_id && next_id != UNINITIALIZED_LONG);

    return NULL;
}
