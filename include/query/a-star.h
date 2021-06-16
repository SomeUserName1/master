#ifndef A_STAR_H
#define A_STAR_H

#include "access/operators.h"
#include "result_types.h"

path*
a_star(in_memory_file_t* db,
       const double*     heuristic,
       unsigned long     source_node_id,
       unsigned long     target_node_id,
       direction_t       direction,
       const char*       log_path);

#endif
