#include "query/degree.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"

size_t
get_degree(heap_file*    hf,
           unsigned long node_id,
           direction_t   direction,
           FILE*         log_file)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        printf("get_degree: Invaliud Arguments!\n");
        exit(EXIT_FAILURE);
    }
    if (log_file) {
        fprintf(log_file, "%s %lu\n", "N ", node_id);
    }

    array_list_relationship* rels   = expand(hf, node_id, direction);
    size_t                   degree = array_list_relationship_size(rels);

    if (log_file) {
        for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
            fprintf(log_file,
                    "%s %lu\n",
                    "R ",
                    array_list_relationship_get(rels, i)->id);
        }
    }

    array_list_relationship_destroy(rels);

    return degree;
}

float
get_avg_degree(heap_file* hf, direction_t direction, FILE* log_file)
{
    if (!hf) {
        printf("get_degree: Invaliud Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t num_nodes    = hf->n_nodes;
    size_t total_degree = 0;

    array_list_relationship* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = expand(hf, i, direction);
        total_degree += array_list_relationship_size(rels);

        if (log_file) {
            fprintf(log_file, "%s %lu\n", "N ", i);

            for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
                fprintf(log_file,
                        "%s %lu\n",
                        "R ",
                        array_list_relationship_get(rels, i)->id);
            }
        }
        array_list_relationship_destroy(rels);
    }

    return ((float)total_degree) / ((float)num_nodes);
}

size_t
get_min_degree(heap_file* hf, direction_t direction)
{
    size_t num_nodes  = hf->n_nodes;
    size_t min_degree = SIZE_MAX;

    array_list_relationship* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = expand(hf, i, direction);
        if (array_list_relationship_size(rels) < min_degree) {
            min_degree = array_list_relationship_size(rels);
        }
        array_list_relationship_destroy(rels);
    }

    return min_degree;
}

size_t
get_max_degree(heap_file* hf, direction_t direction)
{
    size_t num_nodes  = hf->n_nodes;
    size_t max_degree = 0;

    array_list_relationship* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = expand(hf, i, direction);
        if (array_list_relationship_size(rels) > max_degree) {
            max_degree = array_list_relationship_size(rels);
        }
        array_list_relationship_destroy(rels);
    }

    return max_degree;
}
