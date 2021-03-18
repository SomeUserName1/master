#include "random_walk.h"

#include <stdlib.h>
#include <time.h>
#include "../constants.h"

path_t*
random_walk(in_memory_file_t* db,
            unsigned long node_id,
            size_t num_steps,
            direction_t direction)
{
    if (!db || node_id == UNINITIALIZED_LONG) {

    }

    list_ul_t* visited_nodes = create_list_ul();
    list_ul_t* visited_rels = create_list_ul();
    list_relationship_t* cur_rels = in_memory_expand(db, node_id, direction);
    relationship_t* rel;

    list_ul_append(visited_nodes, node_id);
    srand(time(NULL));

    for (size_t i = 0; i < num_steps; ++i) {
        rel = list_relationship_get(cur_rels,
                                    rand() % list_relationship_size(cur_rels));

        node_id =
              rel->source_node == node_id ? rel->target_node : rel->source_node;

        list_ul_append(visited_rels, rel->id);
        list_ul_append(visited_nodes, node_id);
        cur_rels = in_memory_expand(db, node_id, BOTH);
    }

    path_t* path = malloc(sizeof(*path));

    if (!path) {
        exit(-1);
    }

    path->visited_nodes = visited_nodes;
    path->visited_rels = visited_rels;

    return path;
}
