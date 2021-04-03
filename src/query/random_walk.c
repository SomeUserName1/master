#include "random_walk.h"

#include "../constants.h"
#include "result_types.h"

#include <stdio.h>
#include <stdlib.h>

path*
random_walk(in_memory_file_t* db,
            unsigned long     node_id,
            size_t            num_steps,
            direction_t       direction)
{
    if (!db || node_id == UNINITIALIZED_LONG) {
        printf("DB is NULL or node id uninitialized");
    }

    double               distance     = 0.0;
    list_ul_t*           visited_rels = create_list_ul();
    list_relationship_t* cur_rels;
    relationship_t*      rel;
    unsigned long        current_node = node_id;

    for (size_t i = 0; i < num_steps; ++i) {
        cur_rels = in_memory_expand(db, current_node, direction);
        if (list_relationship_size(cur_rels) == 0) {
            list_relationship_destroy(cur_rels);
            break;
        }

        rel = list_relationship_get(cur_rels,
                                    rand() % list_relationship_size(cur_rels));

        list_ul_append(visited_rels, rel->id);
        distance += rel->weight;

        current_node = rel->source_node == current_node ? rel->target_node
                                                        : rel->source_node;
        list_relationship_destroy(cur_rels);
    }

    return create_path(node_id, current_node, distance, visited_rels);
}
