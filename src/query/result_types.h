#ifndef RESULT_TYPES
#define RESULT_TYPES

#include "../data-struct/list_ul.h"

typedef struct traversal_result
{
    unsigned long* traversal_numbers;
    unsigned long* parents;
} traversal_result;

typedef struct sssp_result
{
    double* distances;
    unsigned long* pred_edges;
} sssp_result;

typedef struct path
{
    double distance;
    list_ul_t* edges;
} path;

traversal_result*
create_traversal_result(unsigned long* search_numbers, unsigned long* parents);

void
traversal_result_destroy(traversal_result* result);

sssp_result*
create_sssp_result(double* distances, unsigned long* parents);

void
sssp_result_destroy(sssp_result* result);

path*
create_path(double distance, list_ul_t* edges);

void
path_destroy(path* p);

#endif
