#include "access/in_memory_file.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"

in_memory_file_t*
create_in_memory_file()
{
    in_memory_file_t* file = (in_memory_file_t*)malloc(sizeof(*file));

    if (file == NULL) {
        exit(EXIT_FAILURE);
    }

    file->cache_nodes     = d_ul_node_create();
    file->cache_rels      = d_ul_rel_create();
    file->node_id_counter = 0;
    file->rel_id_counter  = 0;

    return file;
}

void
in_memory_file_destroy(in_memory_file_t* db)
{
    dict_ul_node_destroy(db->cache_nodes);
    dict_ul_rel_destroy(db->cache_rels);
    free(db);
}

unsigned long
in_memory_create_node(in_memory_file_t* db)
{
    if (!db) {
        printf("in_memory - create node: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = new_node();
    node->id     = db->node_id_counter++;

    if (dict_ul_node_insert(db->cache_nodes, node->id, node) < 0) {
        printf("%s", "Inserting the new node failed\n");
        exit(EXIT_FAILURE);
    }

    return node->id;
}

node_t*
in_memory_get_node(in_memory_file_t* db, unsigned long id)
{
    if (!db || id == UNINITIALIZED_LONG) {
        printf("in_memory: get_node: in memory file is NULL or invalid node "
               "requested!\n");
        return NULL;
    }

    return dict_ul_node_get_direct(db->cache_nodes, id);
}

array_list_node*
in_memory_get_nodes(in_memory_file_t* db)
{
    if (!db) {
        printf("in_memory - get_nodes: in memory file is NULL!\n");
        return NULL;
    }

    array_list_node*       nodes = al_node_create();
    unsigned long          id    = UNINITIALIZED_LONG;
    node_t*                node  = NULL;
    dict_ul_node_iterator* it = dict_ul_node_iterator_create(db->cache_nodes);

    while (dict_ul_node_iterator_next(it, &id, &node) > -1) {
        array_list_node_append(nodes, node);
    }
    dict_ul_node_iterator_destroy(it);
    return nodes;
}

relationship_t*
in_memory_get_relationship(in_memory_file_t* db, unsigned long id)
{
    if (!db || id == UNINITIALIZED_LONG) {
        printf("in_memory - get_relationship: in memory file is NULL or "
               "invalid relationship requested!\n");
        return NULL;
    }

    return dict_ul_rel_get_direct(db->cache_rels, id);
}

array_list_relationship*
in_memory_get_relationships(in_memory_file_t* db)
{
    if (!db) {
        printf("in_memory - get_relationships: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    array_list_relationship* rels = al_rel_create();
    unsigned long            id   = UNINITIALIZED_LONG;
    relationship_t*          rel  = NULL;
    dict_ul_rel_iterator*    it   = dict_ul_rel_iterator_create(db->cache_rels);

    while (dict_ul_rel_iterator_next(it, &id, &rel) > -1) {
        array_list_relationship_append(rels, rel);
    }
    dict_ul_rel_iterator_destroy(it);
    return rels;
}

unsigned long
in_memory_create_relationship(in_memory_file_t* db,
                              unsigned long     node_from,
                              unsigned long     node_to)
{
    return in_memory_create_relationship_weighted(db, node_from, node_to, 1.0F);
}

unsigned long
in_memory_create_relationship_weighted(in_memory_file_t* db,
                                       unsigned long     node_from,
                                       unsigned long     node_to,
                                       double            weight)
{
    if (!db || node_from == UNINITIALIZED_LONG || node_to == UNINITIALIZED_LONG
        || weight == UNINITIALIZED_WEIGHT) {
        printf("create_relationship: in memory file is NULL or invalid nodes "
               "or weight passed as "
               "argument!\n");
        printf("in memory file: %p, node from: %lu, node_to %lu, weight "
               "%f\n\t====\n",
               db,
               node_from,
               node_to,
               weight);
        exit(EXIT_FAILURE);
    }
    unsigned long   temp_id;
    relationship_t* last_rel_source  = NULL;
    relationship_t* first_rel_source = NULL;
    relationship_t* last_rel_target  = NULL;
    relationship_t* first_rel_target = NULL;
    node_t*         source_node      = NULL;
    node_t*         target_node      = NULL;

    if (!(source_node = dict_ul_node_get_direct(db->cache_nodes, node_from))
        || !(target_node = dict_ul_node_get_direct(db->cache_nodes, node_to))) {
        printf("%s: %lu, %lu\n",
               "One of the nodes which are refered to by the relationship"
               "to create does not exist:",
               node_from,
               node_to);
        exit(EXIT_FAILURE);
    }

    relationship_t* rel = new_relationship();
    rel->source_node    = node_from;
    rel->target_node    = node_to;
    rel->id             = db->rel_id_counter++;
    rel->flags          = 1;
    rel->weight         = weight;

    // Find first and last relationship in source node's chain
    if (source_node->first_relationship == UNINITIALIZED_LONG) {
        last_rel_source  = rel;
        first_rel_source = rel;
    } else {
        first_rel_source =
              in_memory_get_relationship(db, source_node->first_relationship);

        temp_id = source_node->id == first_rel_source->source_node
                        ? first_rel_source->prev_rel_source
                        : first_rel_source->prev_rel_target;

        last_rel_source = in_memory_get_relationship(db, temp_id);
    }

    // Find first and last relationship in target node's chain
    if (target_node->first_relationship == UNINITIALIZED_LONG) {
        last_rel_target  = rel;
        first_rel_target = rel;
    } else {
        first_rel_target =
              in_memory_get_relationship(db, target_node->first_relationship);

        temp_id = target_node->id == first_rel_target->source_node
                        ? first_rel_target->prev_rel_source
                        : first_rel_target->prev_rel_target;

        last_rel_target = in_memory_get_relationship(db, temp_id);
    }

    // The amount of if clauses significantly decreases when we exclude
    // self-loops.
    // We are altering things after setting the first and last
    // relationships in chains as doing so while figuring out the first and
    // last rels, may produce garbage like when changing a collection during
    // iteration

    // Adjust previous and next pointer for source node in new relation
    rel->prev_rel_source = last_rel_source->id;
    rel->next_rel_source = first_rel_source->id;
    // Adjust previous and next pointer for target node in new relation
    rel->prev_rel_target = last_rel_target->id;
    rel->next_rel_target = first_rel_target->id;

    // Adjust next pointer in source node's previous relation
    if (last_rel_source->source_node == node_from) {
        last_rel_source->next_rel_source = rel->id;
    }
    if (last_rel_source->target_node == node_from) {
        last_rel_source->next_rel_target = rel->id;
    }
    // Adjust next pointer in target node's previous relation
    if (last_rel_target->source_node == node_to) {
        last_rel_target->next_rel_source = rel->id;
    }
    if (last_rel_target->target_node == node_to) {
        last_rel_target->next_rel_target = rel->id;
    }

    // Adjust previous pointer in source node's next relation
    if (first_rel_source->source_node == node_from) {
        first_rel_source->prev_rel_source = rel->id;
    }
    if (first_rel_source->target_node == node_from) {
        first_rel_source->prev_rel_target = rel->id;
    }
    // Adjust previous pointer in target node's next relation
    if (first_rel_target->source_node == node_to) {
        first_rel_target->prev_rel_source = rel->id;
    }
    if (first_rel_target->target_node == node_to) {
        first_rel_target->prev_rel_target = rel->id;
    }

    // Set the first relationship pointer, if the inserted rel is the first rel
    if (source_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_source(rel);
        source_node->first_relationship = rel->id;
    }
    if (target_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_target(rel);
        target_node->first_relationship = rel->id;
    }

    dict_ul_rel_insert(db->cache_rels, rel->id, rel);

    return rel->id;
}

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

    unsigned long next_id  = source_node->first_relationship;
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
              db, source_node->id, rel, direction);
    } while (next_id != start_id && next_id != UNINITIALIZED_LONG);

    return NULL;
}
