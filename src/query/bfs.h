#ifndef BFS_H
#define BFS_H

#include "../data-struct/dict_ul.h"
#include "../access/in_memory_file.h"

typedef struct bfs_result {
    dict_ul_int_t* bfs;
    dict_ul_ul_t* parents;
} bfs_result_t;

bfs_result_t* create_bfs_result(dict_ul_int_t* bfs, dict_ul_ul_t* parents);
void bfs_result_destroy(bfs_result_t* result);

bfs_result_t* bfs(in_memory_file_t* db, unsigned long sourceNodeID, const char* log_path);

#endif
