/*
 * @(#)random_walk.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/random_walk.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "query/result_types.h"

path*
random_walk(heap_file*    hf,
            unsigned long node_id,
            size_t        num_steps,
            direction_t   direction,
            bool          log,
            FILE*         log_file)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        // LCOV_EXCL_START
        printf("DB is NULL or node id uninitialized");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    double                   distance     = 0.0;
    array_list_ul*           visited_rels = al_ul_create();
    array_list_relationship* cur_rels;
    relationship_t*          rel;
    unsigned long            current_node = node_id;

    for (size_t i = 0; i < num_steps; ++i) {
        cur_rels = expand(hf, current_node, direction, log);

        if (log) {
            fprintf(log_file, "random_walk N %lu\n", current_node);
            fflush(log_file);
        }

        if (array_list_relationship_size(cur_rels) == 0) {
            array_list_relationship_destroy(cur_rels);
            break;
        }

        rel = array_list_relationship_get(
              cur_rels, rand() % array_list_relationship_size(cur_rels));
        if (log) {
            fprintf(log_file, "random_walk R %lu\n", rel->id);
            fflush(log_file);
        }

        array_list_ul_append(visited_rels, rel->id);
        distance += rel->weight;

        current_node = rel->source_node == current_node ? rel->target_node
                                                        : rel->source_node;
        array_list_relationship_destroy(cur_rels);
    }

    return create_path(node_id, current_node, distance, visited_rels);
}
