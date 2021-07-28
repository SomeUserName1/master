#include "query/alt.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "query/a-star.h"
#include "query/degree.h"
#include "query/dijkstra.h"
#include "query/result_types.h"

unsigned long
alt_chose_avg_deg_rand_landmark(heap_file*  hf,
                                direction_t direction,
                                const char* log_path)
{
    if (!hf || !log_path) {
        printf("ALT - chose landmarks: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    FILE* log_file = fopen(log_path, "w+");

    if (log_file == NULL) {
        printf("bfs: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }

    double        avg_degree = get_avg_degree(hf, direction, log_file);
    double        degree     = 0;
    unsigned long landmark_id;

    do {
        landmark_id = rand() % hf->n_nodes;
        degree      = (double)get_degree(hf, landmark_id, direction, log_file);
    } while (degree < avg_degree);

    return landmark_id;
}

void
alt_preprocess(heap_file*    hf,
               direction_t   d,
               unsigned long num_landmarks,
               double**      landmark_dists,
               const char*   log_path)
{
    if (!hf || !landmark_dists || !log_path) {
        printf("ALT - preprocess: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long landmarks[num_landmarks];
    sssp_result*  result;

    for (size_t i = 0; i < num_landmarks; ++i) {
        landmarks[i] = alt_chose_avg_deg_rand_landmark(hf, d, log_path);
        result       = dijkstra(hf, landmarks[i], d, log_path);

        // Assign the distances gathered by using dijkstras and discard the rest
        // of the sssp result (pred edges and the struct itself.
        landmark_dists[i] = result->distances;
        free(result->pred_edges);
        free(result);
    }
}

path*
alt(heap_file*    hf,
    double**      landmark_dists,
    unsigned long num_landmarks,
    unsigned long source_node_id,
    unsigned long target_node_id,
    direction_t   direction,
    const char*   log_path)
{
    if (!hf || !landmark_dists || !log_path) {
        printf("ALT: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    double* heuristic = calloc(hf->n_nodes, sizeof(*heuristic));
    if (!heuristic) {
        printf("alt: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    double temp_dist;
    for (size_t i = 0; i < num_landmarks; ++i) {
        for (size_t j = 0; j < hf->n_nodes; ++j) {
            temp_dist = fabs(landmark_dists[i][j]
                             - landmark_dists[i][target_node_id]);
            if (temp_dist < heuristic[j]) {
                heuristic[j] = temp_dist;
            }
        }
    }

    path* result = a_star(
          hf, heuristic, source_node_id, target_node_id, direction, log_path);

    free(heuristic);
    return result;
}
