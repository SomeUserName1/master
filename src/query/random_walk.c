#include "random_walk.h"

#include "../constants.h"
#include "result_types.h"
#include <stdlib.h>
#include <time.h>

path*
random_walk(in_memory_file_t* db,
            unsigned long node_id,
            size_t num_steps,
            direction_t direction)
{
    if (!db || node_id == UNINITIALIZED_LONG) {
    }

    double distance = 0.0;
    list_ul_t* visited_rels = create_list_ul();
    list_relationship_t* cur_rels;
    relationship_t* rel;
    unsigned long current_node = node_id;

    srand(time(NULL));

    for (size_t i = 0; i < num_steps; ++i) {
        cur_rels = in_memory_expand(db, current_node, direction);
        rel = list_relationship_get(cur_rels,
                                    rand() % list_relationship_size(cur_rels));

        list_ul_append(visited_rels, rel->id);
        distance += rel->weight;

        current_node = rel->source_node == current_node ? rel->target_node
                                                        : rel->source_node;
    }

    return create_path(node_id, current_node, distance, visited_rels);
}
