#include "alt.h"

#include "../record/node.h"
#include "degree.h"
#include "dijkstra.h"
#include "a-star.h"

#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

unsigned long
alt_chose_avg_deg_rand_landmark(in_memory_file_t* db, direction_t direction)
{
    if (!db) {
        exit(-1);
    }

    double avg_degree = get_avg_degree(db, direction);
    double degree = 0;
    unsigned long landmark_id;
    node_t* node;

    srand(time(NULL));

    do {
        landmark_id = rand();
        node = in_memory_get_node(db, landmark_id);
        degree = get_degree(db, landmark_id, direction);
    } while (degree < avg_degree);

    return landmark_id;
}

void
alt_preprocess(in_memory_file_t* db,
               direction_t d,
               unsigned long num_landmarks,
               double** landmark_dists,
               const char* log_path)
{
    if (!db || !landmark_dists || !log_path) {
        exit(-1);
    }

    unsigned long landmarks[num_landmarks];
    for (size_t i = 0; i < num_landmarks; ++i) {
        landmarks[i] = alt_chose_avg_deg_rand_landmark(db, d);
        landmark_dists[i] = dijkstra(db, landmarks[i], d, log_path)->distances;
    }
}

path*
alt(in_memory_file_t* db,
    double** landmark_dists,
    unsigned long num_landmarks,
    unsigned long source_node_id,
    unsigned long target_node_id,
    direction_t direction,
    const char* log_path)
{
    if (!db || !landmark_dists || !log_path) {
        exit(-1);
    }

    double* heuristic = calloc(db->node_id_counter, sizeof(*heuristic));
    if (!heuristic) {
        exit(-1);
    }

    double temp_dist;
    for (size_t i = 0; i < num_landmarks; ++i) {
        for (size_t j = 0; j < db->node_id_counter; ++j) {
            temp_dist = fabs(landmark_dists[i][j] -
                             landmark_dists[i][target_node_id]);
            if (temp_dist < heuristic[j]) {
                heuristic[j] = temp_dist;
            }
        }
    }

    return a_star(db, heuristic, source_node_id, target_node_id, direction, log_path);
}
