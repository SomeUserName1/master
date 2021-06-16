#include "query/bfs.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"
#include "access/operators.h"
#include "access/relationship.h"
#include "data-struct/list_rel.h"
#include "data-struct/queue_ul.h"
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
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        parents[i] = UNINITIALIZED_LONG;
        bfs[i]     = ULONG_MAX;
    }

    queue_ul_t* nodes_queue = create_queue_ul();
    FILE*       log_file    = fopen(log_path, "w+");

    if (log_file == NULL) {
        free(parents);
        free(bfs);
        queue_ul_destroy(nodes_queue);
        printf("bfs: Failed to open log file, %d\n", errno);
        exit(-1);
    }

    list_relationship_t* current_rels = NULL;
    relationship_t*      current_rel  = NULL;
    unsigned long        temp;
    unsigned long*       node_id;
    queue_ul_add(nodes_queue, source_node_id);
    bfs[source_node_id] = 0;

    while (queue_ul_size(nodes_queue) > 0) {
        node_id      = queue_ul_take(nodes_queue);
        current_rels = in_memory_expand(db, *node_id, direction);
        fprintf(log_file, "%s %lu\n", "N", *node_id);

        for (size_t i = 0; i < list_relationship_size(current_rels); ++i) {
            current_rel = list_relationship_get(current_rels, i);
            fprintf(log_file, "%s %lu\n", "R", current_rel->id);
            temp = *node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (bfs[temp] == ULONG_MAX) {
                bfs[temp]     = bfs[*node_id] + 1;
                parents[temp] = current_rel->id;
                queue_ul_add(nodes_queue, temp);
            }
        }

        free(node_id);
        list_relationship_destroy(current_rels);
    }
    queue_ul_destroy(nodes_queue);
    fclose(log_file);

    return create_traversal_result(source_node_id, bfs, parents);
}
