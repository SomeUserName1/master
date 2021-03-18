#ifndef RANDOM_WALK
#define RANDOM_WALK
#include "../access/in_memory_file.h"
#include "../data-struct/list_ul.h"

typedef struct path
{
    list_ul_t* visited_nodes;
    list_ul_t* visited_rels;
} path_t;

path_t*
random_walk(in_memory_file_t* db,
            unsigned long node_id,
            size_t num_steps,
            direction_t direction);

#endif
