#include "result_types.h"
#include <stdlib.h>

traversal_result*
create_search_result(unsigned long* traversal_numbers, unsigned long* parents)
{
    if (!traversal_numbers || !parents) {
        exit(-1);
    }

    traversal_result* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->traversal_numbers = traversal_numbers;
    result->parents = parents;

    return result;
}

void
search_result_destroy(traversal_result* result)
{
    if (!result) {
        return;
    }

    free(result->traversal_numbers);
    free(result->parents);
    free(result);
}

sssp_result*
create_sssp_result(double* distances, unsigned long* parents)
{
    if (!distances || !parents) {
        exit(-1);
    }

    sssp_result* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->distances = distances;
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
create_path(double distance, list_ul_t* edges)
{
    if (!distance || !edges) {
        exit(-1);
    }

    path* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->distance = distance;
    result->edges = edges;

    return result;
}

void
path_destroy(path* p)
{
    if (!p) {
        return;
    }

    free(p->edges);
    free(p);
}
