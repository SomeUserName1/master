#include "query/random_walk.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "query/result_types.h"

path*
random_walk(heap_file*    hf,
            unsigned long node_id,
            size_t        num_steps,
            direction_t   direction
#ifdef VERBOSE
            ,
            FILE* log_file
#endif
)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        printf("DB is NULL or node id uninitialized");
    }

    double                   distance     = 0.0;
    array_list_ul*           visited_rels = al_ul_create();
    array_list_relationship* cur_rels;
    relationship_t*          rel;
    unsigned long            current_node = node_id;

    for (size_t i = 0; i < num_steps; ++i) {
        cur_rels = expand(hf, current_node, direction);

#ifdef VERBOSE
        fprintf(log_file, "random_walk N %lu\n", current_node);
#endif

        if (array_list_relationship_size(cur_rels) == 0) {
            array_list_relationship_destroy(cur_rels);
            break;
        }

        rel = array_list_relationship_get(
              cur_rels, rand() % array_list_relationship_size(cur_rels));
#ifdef VERBOSE
        fprintf(log_file, "random_walk R %lu\n", rel->id);
#endif

        array_list_ul_append(visited_rels, rel->id);
        distance += rel->weight;

        current_node = rel->source_node == current_node ? rel->target_node
                                                        : rel->source_node;
        array_list_relationship_destroy(cur_rels);
    }

    return create_path(node_id, current_node, distance, visited_rels);
}
