/*
 * @(#)result_types.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef RESULT_TYPES_H
#define RESULT_TYPES_H

#include "access/heap_file.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"

typedef struct traversal_result
{
    unsigned long source;
    dict_ul_ul*   traversal_numbers;
    dict_ul_ul*   parents;
} traversal_result;

typedef struct sssp_result
{
    unsigned long source;
    dict_ul_d*    distances;
    dict_ul_ul*   pred_edges;
} sssp_result;

typedef struct path
{
    unsigned long  source;
    unsigned long  target;
    double         distance;
    array_list_ul* edges;
} path;

traversal_result*
create_traversal_result(unsigned long source_node,
                        dict_ul_ul*   traversal_numbers,
                        dict_ul_ul*   parents);

void
traversal_result_destroy(traversal_result* result);

sssp_result*
create_sssp_result(unsigned long source_node,
                   dict_ul_d*    distances,
                   dict_ul_ul*   parents);

void
sssp_result_destroy(sssp_result* result);

path*
create_path(unsigned long  source_node_id,
            unsigned long  target_node_id,
            double         distance,
            array_list_ul* edges);

void
path_destroy(path* p);

path*
construct_path(heap_file*    hf,
               unsigned long source_node_id,
               unsigned long target_node_id,
               dict_ul_ul*   parents,
               bool          log);

array_list_ul*
path_extract_vertices(path* p, heap_file* hf, bool log);

#endif
