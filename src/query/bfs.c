#include "query/bfs.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/linked_list.h"
#include "query/in_memory_operators.h"
#include "query/result_types.h"

traversal_result*
bfs(in_memory_file_t* db,
    unsigned long     source_node_id,
    direction_t       direction,
    const char*       log_path)
{
    unsigned long* parents = malloc(db->node_id_counter * sizeof(*parents));
    unsigned long* bfs     = malloc(db->node_id_counter * sizeof(*bfs));

    if (!parents || !bfs) {
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        parents[i] = UNINITIALIZED_LONG;
        bfs[i]     = ULONG_MAX;
    }

    queue_ul* nodes_queue = q_ul_create();
    FILE*     log_file    = fopen(log_path, "w+");

    if (log_file == NULL) {
        free(parents);
        free(bfs);
        queue_ul_destroy(nodes_queue);
        printf("bfs: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }

    array_list_relationship* current_rels = NULL;
    relationship_t*          current_rel  = NULL;
    unsigned long            temp;
    unsigned long            node_id;
    queue_ul_push(nodes_queue, source_node_id);
    bfs[source_node_id] = 0;

    while (queue_ul_size(nodes_queue) > 0) {
        node_id      = queue_ul_pop(nodes_queue);
        current_rels = in_memory_expand(db, node_id, direction);
        fprintf(log_file, "%s %lu\n", "N", node_id);

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);
            fprintf(log_file, "%s %lu\n", "R", current_rel->id);
            temp = node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (bfs[temp] == ULONG_MAX) {
                bfs[temp]     = bfs[node_id] + 1;
                parents[temp] = current_rel->id;
                queue_ul_push(nodes_queue, temp);
            }
        }

        array_list_relationship_destroy(current_rels);
    }
    queue_ul_destroy(nodes_queue);
    fclose(log_file);

    return create_traversal_result(source_node_id, bfs, parents);
}
