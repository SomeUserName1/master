#include "result_types.h"

#include "../constants.h"

#include <stdio.h>
#include <stdlib.h>

traversal_result*
create_traversal_result(unsigned long  source_node,
                        unsigned long* traversal_numbers,
                        unsigned long* parents)
{
    if (!traversal_numbers || !parents) {
        exit(-1);
    }

    traversal_result* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->source            = source_node;
    result->traversal_numbers = traversal_numbers;
    result->parents           = parents;

    return result;
}

void
traversal_result_destroy(traversal_result* result)
{
    if (!result) {
        return;
    }

    free(result->traversal_numbers);
    free(result->parents);
    free(result);
}

sssp_result*
create_sssp_result(unsigned long  source_node,
                   double*        distances,
                   unsigned long* parents)
{
    if (!distances || !parents) {
        exit(-1);
    }

    sssp_result* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->source     = source_node;
    result->distances  = distances;
    result->pred_edges = parents;

    return result;
}

void
sssp_result_destroy(sssp_result* result)
{
    if (!result) {
        return;
    }

    free(result->distances);
    free(result->pred_edges);
    free(result);
}

path*
create_path(unsigned long source_node_id,
            unsigned long target_node_id,
            double        distance,
            list_ul_t*    edges)
{
    if (!edges || source_node_id == UNINITIALIZED_LONG) {
        printf("Tried to create path with null pointer as argument\n");
        exit(-1);
    }

    path* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->source   = source_node_id;
    result->target   = target_node_id;
    result->distance = distance;
    result->edges    = edges;

    return result;
}

void
path_destroy(path* p)
{
    if (!p) {
        return;
    }

    list_ul_destroy(p->edges);
    free(p);
}

list_ul_t*
path_extract_vertices(path* p, in_memory_file_t* db)
{
    list_ul_t* nodes = create_list_ul();

    list_ul_append(nodes, p->source);
    unsigned long   prev_node;
    relationship_t* rel;

    for (size_t i = 0; i < list_ul_size(p->edges); ++i) {
        rel       = in_memory_get_relationship(db, list_ul_get(p->edges, i));
        prev_node = list_ul_get(nodes, list_ul_size(nodes) - 1);

        if (rel->source_node == prev_node) {
            list_ul_append(nodes, rel->target_node);
        } else if (rel->target_node == prev_node) {
            list_ul_append(nodes, rel->source_node);
        }
    }
    return nodes;
}
