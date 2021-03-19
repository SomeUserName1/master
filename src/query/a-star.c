#include "a-star.h"

#include "../access/in_memory_file.h"
#include "../constants.h"
#include "../data-struct/fibonacci_heap.h"
#include "../record/node.h"
#include "../record/relationship.h"
#include "result_types.h"

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

path*
construct_path(in_memory_file_t* db,
               unsigned long source_node_id,
               unsigned long target_node_id,
               unsigned long* parents,
               double distance)
{
    unsigned long node_id = target_node_id;
    list_ul_t* edges_reverse = create_list_ul();
    relationship_t* rel;
    do {
        list_ul_append(edges_reverse, parents[node_id]);

        rel = in_memory_get_relationship(db, parents[node_id]);

        node_id =
              rel->target_node == node_id ? rel->source_node : rel->target_node;
    } while (node_id != source_node_id);

    list_ul_t* edges = create_list_ul();

    for (size_t i = list_ul_size(edges_reverse); i > -1; --i) {
        list_ul_append(edges, list_ul_get(edges_reverse, i));
    }
    list_ul_destroy(edges_reverse);

    return create_path(distance, edges);
}

path*
a_star(in_memory_file_t* db,
       const double* heuristic,
       unsigned long source_node_id,
       unsigned long target_node_id,
       direction_t direction,
       const char* log_path)
{
    unsigned long* parents = malloc(db->node_id_counter * sizeof(*parents));
    double* distance = malloc(db->node_id_counter * sizeof(distance));
    if (!parents || !distance) {
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        parents[i] = UNINITIALIZED_LONG;
        distance[i] = DBL_MAX;
    }

    fib_heap_t* prio_queue = create_fib_heap();
    FILE* log_file = fopen(log_path, "w");

    if (log_file == NULL) {
        free(parents);
        free(distance);
        destroy_fib_heap(prio_queue);
        printf("dijkstra: Failed to open log file, %d\n", errno);
        return NULL;
    }

    list_relationship_t* current_rels;
    relationship_t* current_rel;
    unsigned long temp;
    double new_dist;
    fib_node* fh_node =
          create_fib_node(distance[source_node_id], source_node_id);
    fib_heap_insert(prio_queue, fh_node);

    while (prio_queue->num_nodes > 0) {
        fh_node = fib_heap_extract_min(prio_queue);

        if (fh_node->value == target_node_id) {
            return construct_path(db,
                                  source_node_id,
                                  target_node_id,
                                  parents,
                                  distance[target_node_id]);
        }

        current_rels = in_memory_expand(db, fh_node->value, direction);

        fprintf(log_file, "%s %lu\n", "bfs: Node: ", fh_node->value);

        for (size_t i = 0; i < list_relationship_size(current_rels); ++i) {
            current_rel = list_relationship_get(current_rels, i);

            fprintf(
                  log_file, "%s %lu\n", "bfs: Relationship: ", current_rel->id);

            temp = fh_node->value == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            new_dist = distance[fh_node->value] + current_rel->weight +
                       heuristic[fh_node->value];
            if (distance[temp] > new_dist) {
                distance[temp] = new_dist;
                parents[temp] = current_rel->id;
                fib_heap_insert(prio_queue,
                                create_fib_node(distance[temp], temp));
            }
        }
        free(fh_node);
        list_relationship_destroy(current_rels);
    }
    destroy_fib_heap(prio_queue);
    fclose(log_file);

    return create_path(DBL_MAX, create_list_ul());
}
