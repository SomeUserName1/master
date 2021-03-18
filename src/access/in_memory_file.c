#include "in_memory_file.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/list_node.h"
#include "../data-struct/list_rel.h"
#include "../record/node.h"
#include "../record/relationship.h"

in_memory_file_t*
create_in_memory_file()
{
    in_memory_file_t* file = (in_memory_file_t*)malloc(sizeof(*file));

    if (file == NULL) {
        exit(-1);
    }

    file->cache_nodes = create_dict_ul_node();
    file->cache_rels = create_dict_ul_rel();
    file->node_id_counter = 0;
    file->rel_id_counter = 0;

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
    node_t* node = new_node();
    node->id = db->node_id_counter++;
    node->flags = 1;

    if (dict_ul_node_insert(db->cache_nodes, node->id, node) < 0) {
        printf("%s", "Inserting the new node failed\n");
        exit(-1);
    }

    return node->id;
}

node_t*
in_memory_get_node(in_memory_file_t* db, unsigned long id)
{
    return dict_ul_node_get_direct(db->cache_nodes, id);
}

list_node_t*
in_memory_get_nodes(in_memory_file_t* db)
{
    list_node_t* nodes = create_list_node();
    node_t* node = NULL;
    dict_ul_node_iterator_t* it = create_dict_ul_node_iterator(db->cache_nodes);

    while (dict_ul_node_iterator_next(it, NULL, &node) > -1) {
        list_node_append(nodes, node);
    }
    dict_ul_node_iterator_destroy(it);
    return nodes;
}

relationship_t*
in_memory_get_relationship(in_memory_file_t* db, unsigned long id)
{
    return dict_ul_rel_get_direct(db->cache_rels, id);
}

list_relationship_t*
in_memory_get_relationships(in_memory_file_t* db)
{
    list_relationship_t* rels = create_list_relationship();
    relationship_t* rel = NULL;
    dict_ul_rel_iterator_t* it = create_dict_ul_rel_iterator(db->cache_rels);

    while (dict_ul_rel_iterator_next(it, NULL, &rel) > -1) {
        list_relationship_append(rels, rel);
    }
    dict_ul_rel_iterator_destroy(it);
    return rels;
}

unsigned long
in_memory_create_relationship(in_memory_file_t* db,
                              unsigned long nodeFrom,
                              unsigned long nodeTo)
{
    return in_memory_create_relationship_weighted(db, nodeFrom, nodeTo, 1.0F);
}

unsigned long
in_memory_create_relationship_weighted(in_memory_file_t* db,
                                       unsigned long nodeFrom,
                                       unsigned long nodeTo,
                                       double weight)
{
    if (!dict_ul_node_contains(db->cache_nodes, nodeFrom) ||
        !dict_ul_node_contains(db->cache_nodes, nodeTo)) {
        printf("%s: %lu, %lu\n",
               "One of the nodes which are refered to by the relationship"
               "to create do not exist:",
               nodeFrom,
               nodeTo);
        exit(-1);
    }

    unsigned long next_id;
    relationship_t* rel_source = NULL;
    relationship_t* rel_target = NULL;
    relationship_t* rel = new_relationship();
    node_t* source_node = NULL;
    node_t* target_node = NULL;
    bool first_rel_source = false;
    bool first_rel_target = false;

    rel->source_node = nodeFrom;
    rel->target_node = nodeTo;
    rel->id = db->rel_id_counter++;
    rel->flags = 1;
    rel->weight = weight;

    source_node = dict_ul_node_get_direct(db->cache_nodes, nodeFrom);
    if (source_node->first_relationship == UNINITIALIZED_LONG) {
        first_rel_source = true;
    } else {
        next_id = source_node->first_relationship;

        while (next_id != UNINITIALIZED_LONG) {
            rel_source = dict_ul_rel_get_direct(db->cache_rels, next_id);
            if (rel_source->source_node == nodeFrom &&
                rel_source->target_node == nodeTo) {
                return rel_source->id;
            }
            next_id = in_memory_next_relationship(
                  db, source_node->id, rel_source, BOTH);
        }
    }

    target_node = dict_ul_node_get_direct(db->cache_nodes, nodeTo);
    if (target_node->first_relationship == UNINITIALIZED_LONG) {
        first_rel_target = true;
    } else {
        next_id = target_node->first_relationship;

        while (next_id != UNINITIALIZED_LONG) {
            rel_target = dict_ul_rel_get_direct(db->cache_rels, next_id);
            next_id = in_memory_next_relationship(
                  db, target_node->id, rel_target, BOTH);
        }
    }

    if (first_rel_source) {
        relationship_set_first_source(rel);
        source_node->first_relationship = rel->id;
    } else {
        if (rel_source->source_node == nodeFrom) {
            rel_source->next_rel_source = rel->id;
        }
        if (rel_source->target_node == nodeFrom) {
            rel_source->next_rel_target = rel->id;
        }
        rel->prev_rel_source = rel_source->id;
    }

    if (first_rel_target) {
        relationship_set_first_source(rel);
        target_node->first_relationship = rel->id;
    } else {
        if (rel_target->source_node == nodeTo) {
            rel_target->next_rel_source = rel->id;
        }
        if (rel_target->target_node == nodeTo) {
            rel_target->next_rel_target = rel->id;
        }
        rel->prev_rel_target = rel_target->id;
    }
    dict_ul_rel_insert(db->cache_rels, rel->id, rel);

    return rel->id;
}

unsigned long
in_memory_next_relationship(in_memory_file_t* db,
                            unsigned long node_id,
                            relationship_t* rel,
                            direction_t direction)
{
    unsigned long rel_id = rel->id;

    if (db == NULL || rel == NULL || node_id == UNINITIALIZED_LONG) {
        printf("Arguments must be not NULL!");
        exit(-1);
    }

    while (rel_id != UNINITIALIZED_LONG) {
        if (rel->source_node == node_id &&
            (direction == BOTH || direction == OUTGOING)) {
            return rel->next_rel_source;
        }
        if (rel->target_node == node_id &&
            (direction == BOTH || direction == INCOMING)) {
            return rel->next_rel_target;
        }
        rel_id = node_id == rel->source_node   ? rel->next_rel_source
                 : node_id == rel->target_node ? rel->target_node
                                               : UNINITIALIZED_LONG;
        rel = dict_ul_rel_get_direct(db->cache_rels, rel_id);
    }

    return UNINITIALIZED_LONG;
}

list_relationship_t*
in_memory_expand(in_memory_file_t* db,
                 unsigned long node_id,
                 direction_t direction)
{
    node_t* node = dict_ul_node_get_direct(db->cache_nodes, node_id);
    if (node == NULL || db == NULL) {
        printf("Arguments must be not NULL!");
        exit(-1);
    }
    list_relationship_t* result = create_list_relationship();
    relationship_t* rel = NULL;
    unsigned long rel_id = node->first_relationship;

    while (rel_id != UNINITIALIZED_LONG) {
        rel = dict_ul_rel_get_direct(db->cache_rels, rel_id);
        if ((rel->source_node == node->id && direction != INCOMING) ||
            (rel->target_node == node->id && direction != OUTGOING)) {
            list_relationship_append(result, rel);
        } else {
            printf("%s%lu%s%lu",
                   "Expand: Invalid relationship ",
                   rel->id,
                   " in chain of node ",
                   node->id);
            list_relationship_destroy(result);
            exit(-1);
        }
        rel_id = in_memory_next_relationship(db, node->id, rel, direction);
    }
    return result;
}

relationship_t*
in_memory_contains_relationship_from_to(in_memory_file_t* db,
                                        unsigned long node_from,
                                        unsigned long node_to,
                                        direction_t direction)
{
    relationship_t* rel;
    node_t* source_node = dict_ul_node_get_direct(db->cache_nodes, node_from);
    node_t* target_node = dict_ul_node_get_direct(db->cache_nodes, node_to);

    if (source_node->first_relationship == UNINITIALIZED_LONG ||
        target_node->first_relationship == UNINITIALIZED_LONG) {
        return NULL;
    }
    unsigned long next_id = source_node->first_relationship;

    while (next_id != UNINITIALIZED_LONG) {
        rel = dict_ul_rel_get_direct(db->cache_rels, next_id);
        if ((direction != INCOMING && rel->source_node == node_from &&
             rel->target_node == node_to) ||
            (direction != OUTGOING && rel->source_node == node_to &&
             rel->target_node == node_from)) {
            return rel;
        }
        next_id = in_memory_next_relationship(db, node_from, rel, direction);
    }

    return NULL;
}
