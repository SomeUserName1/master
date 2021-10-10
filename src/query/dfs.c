/*!
 * \file dfs.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref dfs.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/dfs.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "query/result_types.h"
#include "strace.h"

traversal_result*
dfs(heap_file*    hf,
    unsigned long source_node_id,
    direction_t   direction,
    bool          log,
    FILE*         log_file)
{
    if (!hf || source_node_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("dfs: Invalid Arguments!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul* parents = d_ul_ul_create();
    dict_ul_ul* dfs     = d_ul_ul_create();

    array_list_node* nodes = get_nodes(hf, log);
    for (size_t i = 0; i < hf->n_nodes; ++i) {
        dict_ul_ul_insert(dfs, array_list_node_get(nodes, i)->id, ULONG_MAX);
    }
    array_list_node_destroy(nodes);

    stack_ul* node_stack = st_ul_create();

    array_list_relationship* current_rels = NULL;
    relationship_t*          current_rel  = NULL;
    unsigned long            temp;
    unsigned long            node_id;
    stack_ul_push(node_stack, source_node_id);
    dict_ul_ul_insert(dfs, source_node_id, 0);

    size_t stack_size = stack_ul_size(node_stack);

    while (stack_size > 0) {
        node_id      = stack_ul_pop(node_stack);
        current_rels = expand(hf, node_id, direction, log);

        if (log) {
            fprintf(log_file, "dfs %s %lu\n", "N", node_id);
            fflush(log_file);
        }

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);

            if (log) {
                fprintf(log_file, "dfs %s %lu\n", "R", current_rel->id);
                fflush(log_file);
            }

            temp = node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (dict_ul_ul_get_direct(dfs, temp) == ULONG_MAX) {
                dict_ul_ul_insert(
                      dfs, temp, dict_ul_ul_get_direct(dfs, node_id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                stack_ul_push(node_stack, temp);
            }
        }
        stack_size = stack_ul_size(node_stack);
        array_list_relationship_destroy(current_rels);
    }
    stack_ul_destroy(node_stack);

    return create_traversal_result(source_node_id, dfs, parents);
}
