#ifndef A_STAR
#define A_STAR

#include "result_types.h"
#include "../access/in_memory_file.h"

path*
a_star(in_memory_file_t* db,
               const double* heuristic,
               unsigned long source_node_id,
               unsigned long target_node_id,
               direction_t direction,
               const char* log_path);

#endif
