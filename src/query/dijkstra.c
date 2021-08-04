#include "query/dijkstra.h"

#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/fibonacci_heap.h"
#include "data-struct/htable.h"
#include "query/result_types.h"

sssp_result*
dijkstra(heap_file*    hf,
         unsigned long source_node_id,
         direction_t   direction
#ifdef VERBOSE
         ,
         FILE* log_file
#endif
)
{
    if (!hf || source_node_id == UNINITIALIZED_LONG) {
        printf("dijkstra: Invalid Arguemnts!\n");
        exit(EXIT_FAILURE);
    }

    dict_ul_ul* parents  = d_ul_ul_create();
    dict_ul_d*  distance = d_ul_d_create();

    fib_heap_ul* prio_queue = fib_heap_ul_create();

    array_list_relationship* current_rels;
    relationship_t*          current_rel;
    unsigned long            temp;
    double                   new_dist;
    fib_heap_ul_node*        fh_node = NULL;
    fib_heap_ul_insert(prio_queue, DBL_MAX, source_node_id);
    dict_ul_d_insert(distance, source_node_id, 0);

    while (prio_queue->num_nodes > 0) {
        fh_node      = fib_heap_ul_extract_min(prio_queue);
        current_rels = expand(hf, fh_node->value, direction);

#ifdef VERBOSE
        fprintf(log_file, "dijkstra %s %lu\n", "N", fh_node->value);
#endif

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);

#ifdef VERBOSE
            fprintf(log_file, "dijkstra %s %lu\n", "R", current_rel->id);
#endif

            temp = fh_node->value == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            new_dist = dict_ul_d_get_direct(distance, fh_node->value)
                       + current_rel->weight;
            if (dict_ul_d_get_direct(distance, temp) > new_dist) {
                dict_ul_d_insert(distance, temp, new_dist);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                fib_heap_ul_insert(prio_queue, new_dist, temp);
            }
        }
        free(fh_node);
        array_list_relationship_destroy(current_rels);
    }
    fib_heap_ul_destroy(prio_queue);

    return create_sssp_result(source_node_id, distance, parents);
}
