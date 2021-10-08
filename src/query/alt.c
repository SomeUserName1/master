/*!
 * \file alt.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref alt.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/alt.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/heap_file.h"
#include "access/node.h"
#include "data-struct/htable.h"
#include "query/a-star.h"
#include "query/degree.h"
#include "query/dijkstra.h"
#include "query/result_types.h"

unsigned long
alt_chose_avg_deg_rand_landmark(heap_file*  hf,
                                direction_t direction,
                                bool        log,
                                FILE*       log_file)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("ALT - chose landmarks: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    array_list_node* nodes = get_nodes(hf, log);

    double        avg_degree = get_avg_degree(hf, direction, log, log_file);
    double        degree     = 0;
    unsigned long landmark_id;

    do {
        landmark_id = array_list_node_get(nodes, rand() % hf->n_nodes)->id;

        if (log) {
            fprintf(log_file,
                    "alt_chose_avg_deg_rand_landmark N %lu\n",
                    landmark_id);
            fflush(log_file);
        }

        degree = (double)get_degree(hf, landmark_id, direction, log, log_file);
    } while (degree < avg_degree);

    array_list_node_destroy(nodes);

    return landmark_id;
}

void
alt_preprocess(heap_file*    hf,
               direction_t   d,
               unsigned long num_landmarks,
               dict_ul_d**   landmark_dists,
               bool          log,
               FILE*         log_file)
{
    if (!hf || !landmark_dists) {
        // LCOV_EXCL_START
        printf("ALT - preprocess: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long landmarks[num_landmarks];
    sssp_result*  result;

    for (size_t i = 0; i < num_landmarks; ++i) {
        landmarks[i] = alt_chose_avg_deg_rand_landmark(hf, d, log, log_file);
        result       = dijkstra(hf, landmarks[i], d, log, log_file);

        // Assign the distances gathered by using dijkstras and discard the rest
        // of the sssp result (pred edges and the struct itself.
        landmark_dists[i] = result->distances;
        dict_ul_ul_destroy(result->pred_edges);
        free(result);
    }
}

path*
alt(heap_file*    hf,
    dict_ul_d**   landmark_dists,
    unsigned long num_landmarks,
    unsigned long source_node_id,
    unsigned long target_node_id,
    direction_t   direction,
    bool          log,
    FILE*         log_file)
{
    if (!hf || !landmark_dists) {
        // LCOV_EXCL_START
        printf("ALT: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_d*       heuristic = d_ul_d_create();
    array_list_node* nodes     = get_nodes(hf, log);

    double temp_dist;
    for (size_t i = 0; i < num_landmarks; ++i) {
        for (size_t j = 0; j < array_list_node_size(nodes); ++j) {

            temp_dist = fabs(
                  dict_ul_d_get_direct(landmark_dists[i],
                                       array_list_node_get(nodes, j)->id)
                  - dict_ul_d_get_direct(landmark_dists[i], target_node_id));

            if (log) {
                fprintf(log_file,
                        "alt N %lu\n",
                        array_list_node_get(nodes, j)->id);
                fflush(log_file);
            }

            if (!dict_ul_d_contains(heuristic,
                                    array_list_node_get(nodes, j)->id)
                || temp_dist < dict_ul_d_get_direct(
                         heuristic, array_list_node_get(nodes, j)->id)) {
                dict_ul_d_insert(
                      heuristic, array_list_node_get(nodes, j)->id, temp_dist);
            }
        }
    }

    array_list_node_destroy(nodes);

    path* result = a_star(hf,
                          heuristic,
                          source_node_id,
                          target_node_id,
                          direction,
                          log,
                          log_file);

    dict_ul_d_destroy(heuristic);
    return result;
}
