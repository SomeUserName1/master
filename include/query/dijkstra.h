#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

sssp_result*
dijkstra(heap_file*    hf,
         unsigned long source_node_id,
         direction_t   direction,
         const char*   log_path);

#endif
