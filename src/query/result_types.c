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
        exit(EXIT_FAILURE);
    }

    traversal_result* result = malloc(sizeof(*result));

    if (!result) {
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    sssp_result* result = malloc(sizeof(*result));

    if (!result) {
        exit(EXIT_FAILURE);
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
        printf("Tried to create path with null pointer as argument\n");
        exit(EXIT_FAILURE);
    }

    path* result = malloc(sizeof(*result));

    if (!result) {
        exit(EXIT_FAILURE);
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

    array_list_ul_destroy(p->edges);
    free(p);
}

path*
construct_path(heap_file*    hf,
               unsigned long source_node_id,
               unsigned long target_node_id,
               dict_ul_ul*   parents,
               double        distance,
               FILE*         log_file)
{
    unsigned long   node_id       = target_node_id;
    array_list_ul*  edges_reverse = al_ul_create();
    relationship_t* rel;
    unsigned long   parent_id;
    do {
        parent_id = dict_ul_ul_get_direct(parents, node_id);
        array_list_ul_append(edges_reverse, parent_id);
        rel = read_relationship(hf, parent_id);
        fprintf(log_file, "%s %lu\n", "R", rel->id);

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
    fclose(log_file);

    return create_path(source_node_id, target_node_id, distance, edges);
}

array_list_ul*
path_extract_vertices(path* p, heap_file* hf)
{
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
