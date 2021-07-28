#ifndef A_STAR_H
#define A_STAR_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

path*
a_star(heap_file*    hf,
       const double* heuristic,
       unsigned long source_node_id,
       unsigned long target_node_id,
       direction_t   direction,
       const char*   log_path);

#endif
