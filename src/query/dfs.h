#ifndef DFS
#define DFS

#include "../access/in_memory_file.h"
#include "result_types.h"

search_result_t*
dfs(in_memory_file_t* db,
    unsigned long source_node_id,
    direction_t direction,
    const char* log_path);

#endif
