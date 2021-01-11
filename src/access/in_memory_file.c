#include "in_memory_file.h"

#include <stdio.h>
#include <stdlib.h>

#include "../data-struct/dict_ul.h"
#include "../data-struct/list.h"
#include "../data-struct/list_node.h"
#include "../data-struct/list_rel.h"
#include "../record/node.h"
#include "../record/relationship.h"

in_memory_file_t* create_in_memory_file() {
    in_memory_file_t* file = (in_memory_file_t*) malloc(sizeof(*file));

    file->cache_nodes = create_dict_ul_node();
    file->cache_rels = create_dict_ul_rel();

    return file;
}

void in_memory_file_destroy(in_memory_file_t* db) {
    dict_ul_node_destroy(db->cache_nodes);
    dict_ul_rel_destroy(db->cache_rels);
    free(db);
}

int create_node(in_memory_file_t* db, unsigned long id) {
    if (id == UNINITIALIZED_LONG) {
        printf("%s", "Node ID out of range: can only store 2^64 - 1 nodes\n");
        return -1;
    }
    if (dict_ul_node_contains(db->cache_nodes, id)) {
        return 0;
    }

    node_t* node = new_node();
    node->id = id;
    node->flags = 1;

    if (dict_ul_node_insert(db->cache_nodes, id, node) < 0) {
        printf("%s", "Inserting the new node failed\n");
        return -1;
    }

    return 0;
}

int create_relationship(in_memory_file_t* db, unsigned long nodeFrom, unsigned long nodeTo) {
    if (!dict_ul_node_contains(db->cache_nodes, nodeFrom)
            || !dict_ul_node_contains(db->cache_nodes, nodeTo)) {
        printf("%s", "One of the nodes which are refered to by the relationship to create do not exist!\n");
        return -1;
    }

    relationship_t* next;
    relationship_t* rel = new_relationship();
    node_t* firstNode;
    node_t* secondNode;

    rel->source_node = nodeFrom;
    rel->target_node = nodeTo;
    rel->id = db->rel_id_counter++;
    rel->flags = 1;

    firstNode = dict_ul_node_get_direct(db->cache_nodes, nodeFrom);
    if (firstNode->first_relationship == UNINITIALIZED_LONG) {
        dict_ul_rel_insert(db->cache_rels, rel->id, rel);
        firstNode->first_relationship = rel->id;
    } else {
        next = dict_ul_rel_get_direct(db->cache_rels, firstNode->first_relationship);

        do {
            if (next->source_node == nodeFrom && next->target_node == nodeTo) {
                return 0;
            }
            next = dict_ul_rel_get_direct(db->cache_rels, next->next_rel_source);
        } while(next->next_rel_source != UNINITIALIZED_LONG);

        dict_ul_rel_insert(db->cache_rels, rel->id, rel);
        next->next_rel_source = rel->id;
        rel->prev_rel_source = next->id;
    }

    secondNode = dict_ul_node_get_direct(db->cache_nodes, nodeTo);
    if (secondNode->first_relationship == UNINITIALIZED_LONG) {
        secondNode->first_relationship = rel->id;
    } else {
        do {
            next = dict_ul_rel_get_direct(db->cache_rels, next->next_rel_source);
        } while(next->next_rel_source != UNINITIALIZED_LONG);

        next->next_rel_target = rel->id;
        rel->prev_rel_target = next->id;
    }

    return 0;
}


node_t* in_memory_get_node(in_memory_file_t* db, unsigned long id) {
    return  dict_ul_node_get_direct(db->cache_nodes, id);
}

list_node_t* in_memory_get_nodes(in_memory_file_t* db) {
    list_node_t* nodes = create_list_node(LIST_NONE);
    node_t* node = NULL;
    dict_ul_node_iterator_t* it = create_dict_ul_node_iterator(db->cache_nodes);

    while(dict_ul_node_iterator_next(it, NULL, &node) > -1) {
           list_node_append(nodes, node);
    }
    return nodes;
}

relationship_t* in_memory_get_relationship(in_memory_file_t* db, unsigned long id) {
    return dict_ul_rel_get_direct(db->cache_rels, id);
}

list_relationship_t* in_memory_get_relationships(in_memory_file_t* db) {
    list_relationship_t* rels = create_list_relationship(LIST_NONE);
    relationship_t* rel = NULL;
    dict_ul_rel_iterator_t* it = create_dict_ul_rel_iterator(db->cache_rels);

    while(dict_ul_rel_iterator_next(it, NULL, &rel) > -1) {
           list_relationship_append(rels, rel);
    }
    return rels;
}

