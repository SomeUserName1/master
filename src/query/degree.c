#include "degree.h"

#include <stdint.h>
#include <stdio.h>

#include "../access/in_memory_file.h"
#include "../data-struct/list_rel.h"
#include "../record/relationship.h"

size_t
get_degree(in_memory_file_t* db,
           unsigned long     node_id,
           direction_t       direction,
           FILE*             log_file)
{
    list_relationship_t* rels   = in_memory_expand(db, node_id, direction);
    size_t               degree = list_relationship_size(rels);

    if (log_file) {
        fprintf(log_file, "%s %lu\n", "N ", node_id);

        for (size_t i = 0; i < list_relationship_size(rels); ++i) {
            fprintf(log_file,
                    "%s %lu\n",
                    "R ",
                    list_relationship_get(rels, i)->id);
        }
    }

    list_relationship_destroy(rels);

    return degree;
}

float
get_avg_degree(in_memory_file_t* db, direction_t direction, FILE* log_file)
{
    size_t               num_nodes    = db->node_id_counter;
    size_t               total_degree = 0;
    list_relationship_t* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, direction);
        total_degree += list_relationship_size(rels);

        if (log_file) {
            fprintf(log_file, "%s %lu\n", "N ", i);

            for (size_t i = 0; i < list_relationship_size(rels); ++i) {
                fprintf(log_file,
                        "%s %lu\n",
                        "R ",
                        list_relationship_get(rels, i)->id);
            }
        }
        list_relationship_destroy(rels);
    }

    return ((float)total_degree) / ((float)num_nodes);
}

size_t
get_min_degree(in_memory_file_t* db, direction_t direction)
{
    size_t               num_nodes  = db->node_id_counter;
    size_t               min_degree = SIZE_MAX;
    list_relationship_t* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, direction);
        if (list_relationship_size(rels) < min_degree) {
            min_degree = list_relationship_size(rels);
        }
        list_relationship_destroy(rels);
    }

    return min_degree;
}

size_t
get_max_degree(in_memory_file_t* db, direction_t direction)
{
    size_t               num_nodes  = db->node_id_counter;
    size_t               max_degree = 0;
    list_relationship_t* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, direction);
        if (list_relationship_size(rels) > max_degree) {
            max_degree = list_relationship_size(rels);
        }
        list_relationship_destroy(rels);
    }

    return max_degree;
}
