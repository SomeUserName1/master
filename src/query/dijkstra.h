#ifndef DIJKSTRA
#define DIJKSTRA

#include "bfs.h"
typedef struct bfs_result dijkstra_result_t;

dijkstra_result_t* create_dijkstra_result(dict_ul_int_t* dist, dict_ul_ul_t* parents);
void dijkstra_result_destroy(dijkstra_result_t* result);

dijkstra_result_t* dijkstra(in_memory_file_t* db, unsigned long sourceNodeID, const char* log_path)
#endif
