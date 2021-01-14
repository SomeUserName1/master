#ifndef __BFS_H__
#define __BFS_H__

#include "../data-struct/dict_ul.h"
#include "../data-struct/list_ul.h"
#include "../access/in_memory_file.h"

#include <stdio.h>

list_ul_t* construct_path(in_memory_file_t* db, unsigned long source, unsigned long target, dict_ul_ul_t* parents, FILE* log_file);

list_ul_t* bfs(in_memory_file_t* db, unsigned long sourceNodeID, unsigned long targetNodeID, const char* log_path);

#endif
