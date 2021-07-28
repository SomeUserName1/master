#include "query/a-star.h"

#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/fibonacci_heap.h"
#include "query/result_types.h"

path*
a_star(heap_file*    hf,
       const double* heuristic,
       unsigned long source_node_id,
       unsigned long target_node_id,
       direction_t   direction,
       const char*   log_path)
{
    if (!hf || source_node_id == UNINITIALIZED_LONG
        || target_node_id == UNINITIALIZED_LONG || !log_path) {
        printf("a-star: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long* parents  = malloc(hf->n_nodes * sizeof(*parents));
    double*        distance = malloc(hf->n_nodes * sizeof(*distance));

    if (!parents || !distance) {
        printf("a-star: memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        parents[i]  = UNINITIALIZED_LONG;
        distance[i] = DBL_MAX;
    }

    fib_heap_ul* prio_queue = fib_heap_ul_create();
    FILE*        log_file   = fopen(log_path, "w+");

    if (log_file == NULL) {
        free(parents);
        free(distance);
        fib_heap_ul_destroy(prio_queue);
        printf("dijkstra: Failed to open log file, %d\n", errno);
        return NULL;
    }

    array_list_relationship* current_rels;
    relationship_t*          current_rel;
    unsigned long            temp;
    double                   new_dist;
    fib_heap_ul_node*        fh_node = NULL;
    fib_heap_ul_insert(prio_queue, distance[source_node_id], source_node_id);
    distance[source_node_id] = 0;

    while (prio_queue->num_nodes > 0) {
        fh_node = fib_heap_ul_extract_min(prio_queue);

        if (fh_node->value == target_node_id) {
            new_dist = distance[target_node_id];
            free(distance);
            free(fh_node);
            fib_heap_ul_destroy(prio_queue);
            return construct_path(hf,
                                  source_node_id,
                                  target_node_id,
                                  parents,
                                  new_dist,
                                  log_file);
        }

        current_rels = expand(hf, fh_node->value, direction);

        fprintf(log_file, "%s %lu\n", "N", fh_node->value);

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);

            fprintf(log_file, "%s %lu\n", "R", current_rel->id);

            temp = fh_node->value == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            new_dist = distance[fh_node->value] + current_rel->weight
                       + heuristic[fh_node->value];
            if (distance[temp] > new_dist) {
                distance[temp] = new_dist;
                parents[temp]  = current_rel->id;
                fib_heap_ul_insert(prio_queue, distance[temp], temp);
            }
        }
        free(fh_node);
        array_list_relationship_destroy(current_rels);
    }
    fib_heap_ul_destroy(prio_queue);
    free(parents);
    free(distance);
    fclose(log_file);

    return create_path(source_node_id, target_node_id, DBL_MAX, al_ul_create());
}
