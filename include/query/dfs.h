#ifndef DFS_H
#define DFS_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

traversal_result*
dfs(heap_file*    hf,
    unsigned long source_node_id,
    direction_t   direction,
    FILE*         log_file);

#endif
