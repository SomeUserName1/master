#ifndef RESULT_TYPES_H
#define RESULT_TYPES_H

#include <stdio.h>

#include "access/operators.h"
#include "data-struct/array_list.h"

typedef struct traversal_result
{
    unsigned long  source;
    unsigned long* traversal_numbers;
    unsigned long* parents;
} traversal_result;

typedef struct sssp_result
{
    unsigned long  source;
    double*        distances;
    unsigned long* pred_edges;
} sssp_result;

typedef struct path
{
    unsigned long  source;
    unsigned long  target;
    double         distance;
    array_list_ul* edges;
} path;

traversal_result*
create_traversal_result(unsigned long  source_node,
                        unsigned long* traversal_numbers,
                        unsigned long* parents);

void
traversal_result_destroy(traversal_result* result);

sssp_result*
create_sssp_result(unsigned long  source_node,
                   double*        distances,
                   unsigned long* parents);

void
sssp_result_destroy(sssp_result* result);

path*
sssp_extract_path(sssp_result* result, in_memory_file_t db);

path*
create_path(unsigned long  source_node_id,
            unsigned long  target_node_id,
            double         distance,
            array_list_ul* edges);

void
path_destroy(path* p);

path*
construct_path(in_memory_file_t* db,
               unsigned long     source_node_id,
               unsigned long     target_node_id,
               unsigned long*    parents,
               double            distance,
               FILE*             log_file);

array_list_ul*
path_extract_vertices(path* p, in_memory_file_t* db);

#endif
