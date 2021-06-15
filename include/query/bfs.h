#ifndef BFS_H
#define BFS_H

#include "../access/in_memory_file.h"
#include "result_types.h"

traversal_result*
bfs(in_memory_file_t* db,
    unsigned long     source_node_id,
    direction_t       direction,
    const char*       log_path);

#endif
