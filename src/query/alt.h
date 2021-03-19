#ifndef ALT
#define ALT

#include "../access/in_memory_file.h"
#include "result_types.h"

void
alt_preprocess(in_memory_file_t* db,
               direction_t d,
               unsigned long num_landmarks,
               double** landmark_dists,
               const char* log_path);

path*
alt(in_memory_file_t* db,
    double** landmark_dists,
    unsigned long num_landmarks,
    unsigned long source_node_id,
    unsigned long target_node_id,
    direction_t direction,
    const char* log_path);

#endif
