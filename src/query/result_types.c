/*
 * @(#)result_types.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/result_types.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"

traversal_result*
create_traversal_result(unsigned long source_node,
                        dict_ul_ul*   traversal_numbers,
                        dict_ul_ul*   parents)
{
    if (!traversal_numbers || !parents) {
        // LCOV_EXCL_START
        printf("result types - create traversal result: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    traversal_result* result = malloc(sizeof(*result));

    if (!result) {
        // LCOV_EXCL_START
        printf("result types - create traversal result: Failed to allocate "
               "memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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

    dict_ul_ul_destroy(result->traversal_numbers);
    dict_ul_ul_destroy(result->parents);
    free(result);
}

sssp_result*
create_sssp_result(unsigned long source_node,
                   dict_ul_d*    distances,
                   dict_ul_ul*   parents)
{
    if (!distances || !parents) {
        // LCOV_EXCL_START
        printf("result types - create sssp result: Invalid Arguments\n\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    sssp_result* result = malloc(sizeof(*result));

    if (!result) {
        // LCOV_EXCL_START
        printf("result types - create sssp result: Failed to allocate "
               "memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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

    dict_ul_d_destroy(result->distances);
    dict_ul_ul_destroy(result->pred_edges);
    free(result);
}

path*
create_path(unsigned long  source_node_id,
            unsigned long  target_node_id,
            double         distance,
            array_list_ul* edges)
{
    if (!edges || source_node_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("result types - create path: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    path* result = malloc(sizeof(*result));

    if (!result) {
        // LCOV_EXCL_START
        printf("result types - create path: Failed to allocate "
               "memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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
        // LCOV_EXCL_LINE
        return;
    }

    array_list_ul_destroy(p->edges);
    free(p);
}

path*
construct_path(heap_file*    hf,
               unsigned long source_node_id,
               unsigned long target_node_id,
               dict_ul_ul*   parents,
               double        distance
#ifdef VERBOSE
               ,
               FILE* log_file
#endif
)
{
    if (!hf || source_node_id == UNINITIALIZED_LONG
        || target_node_id == UNINITIALIZED_LONG || !parents) {
        // LCOV_EXCL_START
        printf("result types - construct path: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long   node_id       = target_node_id;
    array_list_ul*  edges_reverse = al_ul_create();
    relationship_t* rel;
    unsigned long   parent_id;
    do {
        parent_id = dict_ul_ul_get_direct(parents, node_id);
        array_list_ul_append(edges_reverse, parent_id);
        rel = read_relationship(hf, parent_id);

#ifdef VERBOSE
        fprintf(log_file, "construct_path %s %lu\n", "R", rel->id);
#endif

        node_id =
              rel->target_node == node_id ? rel->source_node : rel->target_node;
    } while (node_id != source_node_id);

    array_list_ul* edges = al_ul_create();

    for (size_t i = 1; i < array_list_ul_size(edges_reverse) + 1; ++i) {
        array_list_ul_append(
              edges,
              array_list_ul_get(edges_reverse,
                                array_list_ul_size(edges_reverse) - i));
    }
    array_list_ul_destroy(edges_reverse);
    dict_ul_ul_destroy(parents);

    return create_path(source_node_id, target_node_id, distance, edges);
}

array_list_ul*
path_extract_vertices(path* p, heap_file* hf)
{
    if (!hf || !p) {
        // LCOV_EXCL_START
        printf("result types - path extract vertices: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_ul* nodes = al_ul_create();

    array_list_ul_append(nodes, p->source);
    unsigned long   prev_node;
    relationship_t* rel;

    for (size_t i = 0; i < array_list_ul_size(p->edges); ++i) {
        rel       = read_relationship(hf, array_list_ul_get(p->edges, i));
        prev_node = array_list_ul_get(nodes, array_list_ul_size(nodes) - 1);

        if (rel->source_node == prev_node) {
            array_list_ul_append(nodes, rel->target_node);
        } else if (rel->target_node == prev_node) {
            array_list_ul_append(nodes, rel->source_node);
        }
    }

    return nodes;
}
