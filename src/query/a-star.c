/*!
 * \file a-star.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref a-star.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/a-star.h"

#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/fibonacci_heap.h"
#include "data-struct/htable.h"
#include "query/result_types.h"

path*
a_star(heap_file*    hf,
       dict_ul_d*    heuristic,
       unsigned long source_node_id,
       unsigned long target_node_id,
       direction_t   direction,
       bool          log,
       FILE*         log_file)
{
    if (!hf
        || source_node_id == UNINITIALIZED_LONG
        // LCOV_EXCL_START
        || target_node_id == UNINITIALIZED_LONG) {
        printf("a-star: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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
        fh_node = fib_heap_ul_extract_min(prio_queue);

        if (fh_node->value == target_node_id) {
            dict_ul_d_destroy(distance);
            free(fh_node);
            fib_heap_ul_destroy(prio_queue);
            return construct_path(
                  hf, source_node_id, target_node_id, parents, log_file);
        }

        current_rels = expand(hf, fh_node->value, direction, log);

        if (log) {
            fprintf(log_file, "%s %lu\n", "a-star N", fh_node->value);
            fflush(log_file);
        }

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);

            if (log) {
                fprintf(log_file, "%s %lu\n", "a-star R", current_rel->id);
                fflush(log_file);
            }
            temp = fh_node->value == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            new_dist = dict_ul_d_get_direct(distance, fh_node->value)
                       + current_rel->weight
                       + dict_ul_d_get_direct(heuristic, fh_node->value);
            if (!dict_ul_d_contains(distance, temp)
                || dict_ul_d_get_direct(distance, temp) > new_dist) {
                dict_ul_d_insert(distance, temp, new_dist);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                fib_heap_ul_insert(prio_queue, new_dist, temp);
            }
        }
        free(fh_node);
        array_list_relationship_destroy(current_rels);
    }
    // LCOV_EXCL_START
    fib_heap_ul_destroy(prio_queue);
    dict_ul_ul_destroy(parents);
    dict_ul_d_destroy(distance);

    return create_path(source_node_id, target_node_id, DBL_MAX, al_ul_create());
    // LCOV_EXCL_STOP
}
