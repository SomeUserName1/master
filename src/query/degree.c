#include "degree.h"

#include <stdint.h>

#include "../data-struct/list_node.h"
#include "../data-struct/list_rel.h"

float get_avg_degree(in_memory_file_t* db, direction_t direction) {
    list_node_t* nodes = in_memory_get_nodes(db);
    size_t num_nodes = list_node_size(nodes);
    size_t total_degree = 0;
    list_relationship_t* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, direction);
        total_degree += list_relationship_size(rels);
    }

    return ((float) total_degree) / ((float) num_nodes);
}

size_t get_min_degree(in_memory_file_t* db, direction_t direction) {
    list_node_t* nodes = in_memory_get_nodes(db);
    size_t num_nodes = list_node_size(nodes);
    size_t min_degree = SIZE_MAX;
    list_relationship_t* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, direction);
        if (list_relationship_size(rels) < min_degree) {
            min_degree = list_relationship_size(rels);
        }
    }

    return min_degree;
}

size_t get_max_degree(in_memory_file_t* db, direction_t direction) {
    list_node_t* nodes = in_memory_get_nodes(db);
    size_t num_nodes = list_node_size(nodes);
    size_t max_degree = 0;
    list_relationship_t* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, direction);
        if (list_relationship_size(rels) > max_degree) {
            max_degree = list_relationship_size(rels);
        }
    }

    return max_degree;
}
