#include "query/degree.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/node.h"
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

#ifdef VERBOSE
    if (log_file) {
        fprintf(log_file, "get_degree %s %lu\n", "N ", node_id);
    }
#endif

    array_list_relationship* rels   = expand(hf, node_id, direction);
    size_t                   degree = array_list_relationship_size(rels);

    if (log_file) {
#ifdef VERBOSE
        for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {

            fprintf(log_file,
                    "get_degree %s %lu\n",
                    "R ",
                    array_list_relationship_get(rels, i)->id);
        }
#endif
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

    array_list_node* nodes        = get_nodes(hf);
    size_t           num_nodes    = array_list_node_size(nodes);
    size_t           total_degree = 0;

    array_list_relationship* rels;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = expand(hf, array_list_node_get(nodes, i)->id, direction);
        total_degree += array_list_relationship_size(rels);

        if (log_file) {
#ifdef VERBOSE
            fprintf(log_file, "get_avg_degree %s %lu\n", "N ", i);

            for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
                fprintf(log_file,
                        "get_avg_degree %s %lu\n",
                        "R ",
                        array_list_relationship_get(rels, i)->id);
            }
#endif
        }
        array_list_relationship_destroy(rels);
    }

    return ((float)total_degree) / ((float)num_nodes);
}

size_t
get_min_degree(heap_file* hf, direction_t direction, FILE* log_file)
{
    if (!hf) {
        printf("degree - get min degree: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    array_list_node* nodes      = get_nodes(hf);
    size_t           num_nodes  = array_list_node_size(nodes);
    size_t           min_degree = SIZE_MAX;
    size_t           degree;

    for (size_t i = 0; i < num_nodes; ++i) {
        degree = get_degree(
              hf, array_list_node_get(nodes, i)->id, direction, log_file);

        if (degree < min_degree) {
            min_degree = degree;
        }
    }
    array_list_node_destroy(nodes);

    return min_degree;
}

size_t
get_max_degree(heap_file* hf, direction_t direction, FILE* log_file)
{
    if (!hf) {
        printf("degree - get max degree: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    array_list_node* nodes      = get_nodes(hf);
    size_t           num_nodes  = array_list_node_size(nodes);
    size_t           max_degree = 0;
    size_t           degree;

    for (size_t i = 0; i < num_nodes; ++i) {
        degree = get_degree(
              hf, array_list_node_get(nodes, i)->id, direction, log_file);

        if (degree > max_degree) {
            max_degree = degree;
        }
    }

    array_list_node_destroy(nodes);

    return max_degree;
}
