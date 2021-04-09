#include "alt.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../access/in_memory_file.h"
#include "a-star.h"
#include "degree.h"
#include "dijkstra.h"
#include "result_types.h"

unsigned long
alt_chose_avg_deg_rand_landmark(in_memory_file_t* db,
                                direction_t       direction,
                                const char*       log_path)
{
    if (!db || !log_path) {
        printf("ALT chose landmarks: Invalid Arguments!\n");
        exit(-1);
    }

    FILE* log_file = fopen(log_path, "w+");

    if (log_file == NULL) {
        printf("bfs: Failed to open log file, %d\n", errno);
        exit(-1);
    }

    double        avg_degree = get_avg_degree(db, direction, log_file);
    double        degree     = 0;
    unsigned long landmark_id;

    do {
        landmark_id = rand() % db->node_id_counter;
        degree      = get_degree(db, landmark_id, direction, log_file);
    } while (degree < avg_degree);

    return landmark_id;
}

void
alt_preprocess(in_memory_file_t* db,
               direction_t       d,
               unsigned long     num_landmarks,
               double**          landmark_dists,
               const char*       log_path)
{
    if (!db || !landmark_dists || !log_path) {
        printf("ALT preprocess: Invalid arguments!\n");
        exit(-1);
    }

    unsigned long landmarks[num_landmarks];
    sssp_result*  result;

    for (size_t i = 0; i < num_landmarks; ++i) {
        landmarks[i] = alt_chose_avg_deg_rand_landmark(db, d, log_path);
        result       = dijkstra(db, landmarks[i], d, log_path);

        // Assign the distances gathered by using dijkstras and discard the rest
        // of the sssp result (pred edges and the struct itself.
        landmark_dists[i] = result->distances;
        free(result->pred_edges);
        free(result);
    }
}

path*
alt(in_memory_file_t* db,
    double**          landmark_dists,
    unsigned long     num_landmarks,
    unsigned long     source_node_id,
    unsigned long     target_node_id,
    direction_t       direction,
    const char*       log_path)
{
    if (!db || !landmark_dists || !log_path) {
        printf("ALT: Invalid arguments!\n");
        exit(-1);
    }

    double* heuristic = calloc(db->node_id_counter, sizeof(*heuristic));
    if (!heuristic) {
        exit(-1);
    }

    double temp_dist;
    for (size_t i = 0; i < num_landmarks; ++i) {
        for (size_t j = 0; j < db->node_id_counter; ++j) {
            temp_dist = fabs(landmark_dists[i][j]
                             - landmark_dists[i][target_node_id]);
            if (temp_dist < heuristic[j]) {
                heuristic[j] = temp_dist;
            }
        }
    }

    path* result = a_star(
          db, heuristic, source_node_id, target_node_id, direction, log_path);

    free(heuristic);
    return result;
}
