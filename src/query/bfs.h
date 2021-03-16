#ifndef BFS_H
#define BFS_H

#include "../access/in_memory_file.h"
#include "result_types.h"

search_result_t* bfs(in_memory_file_t* db, unsigned long source_node_id, direction_t direction, const char* log_path);

#endif
