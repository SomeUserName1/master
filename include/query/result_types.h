#ifndef RESULT_TYPES_H
#define RESULT_TYPES_H

#include "access/operators.h"
#include "data-struct/list_ul.h"

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
    unsigned long source;
    unsigned long target;
    double        distance;
    list_ul_t*    edges;
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
create_path(unsigned long source_node_id,
            unsigned long target_node_id,
            double        distance,
            list_ul_t*    edges);

void
path_destroy(path* p);

list_ul_t*
path_extract_vertices(path* p, in_memory_file_t* db);

#endif
