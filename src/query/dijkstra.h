#ifndef DIJKSTRA
#define DIJKSTRA

#include "../access/in_memory_file.h"
#include "result_types.h"

sssp_result*
dijkstra(in_memory_file_t* db,
        unsigned long source_node_id,
        direction_t direction,
        const char* log_path);

#endif
