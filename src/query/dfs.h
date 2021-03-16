#ifndef DFS
#define DFS

#include "result_types.h"
#include "../access/in_memory_file.h"

search_result_t*
dfs(in_memory_file_t* db, unsigned long source_node_id, direction_t direction, const char* log_path);


#endif
