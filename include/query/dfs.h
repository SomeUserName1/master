#ifndef DFS_H
#define DFS_H

#include "access/in_memory_file.h"
#include "access/relationship.h"
#include "result_types.h"

traversal_result*
dfs(in_memory_file_t* db,
    unsigned long     source_node_id,
    direction_t       direction,
    const char*       log_path);

#endif
