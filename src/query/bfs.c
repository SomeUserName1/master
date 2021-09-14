/*
 * @(#)bfs.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/bfs.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "query/result_types.h"

traversal_result*
bfs(heap_file*    hf,
    unsigned long source_node_id,
    direction_t   direction
#ifdef VERBOSE
    ,
    FILE* log_file
#endif
)
{
    if (!hf || source_node_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("bfs: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul* parents = d_ul_ul_create();
    dict_ul_ul* bfs     = d_ul_ul_create();

    queue_ul* nodes_queue = q_ul_create();

    array_list_relationship* current_rels = NULL;
    relationship_t*          current_rel  = NULL;
    unsigned long            temp;
    unsigned long            node_id;
    queue_ul_push(nodes_queue, source_node_id);
    dict_ul_ul_insert(bfs, source_node_id, 0);

    while (queue_ul_size(nodes_queue) > 0) {
        node_id      = queue_ul_pop(nodes_queue);
        current_rels = expand(hf, node_id, direction);

#ifdef VERBOSE
        fprintf(log_file, "bfs %s %lu\n", "N", node_id);
#endif

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);

#ifdef VERBOSE
            fprintf(log_file, "bfs %s %lu\n", "R", current_rel->id);
#endif

            temp = node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (!dict_ul_ul_contains(bfs, temp)) {
                dict_ul_ul_insert(
                      bfs, temp, dict_ul_ul_get_direct(bfs, node_id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                queue_ul_push(nodes_queue, temp);
            }
        }

        array_list_relationship_destroy(current_rels);
    }
    queue_ul_destroy(nodes_queue);

    return create_traversal_result(source_node_id, bfs, parents);
}
