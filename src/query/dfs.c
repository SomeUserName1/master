#include "dfs.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"
#include "../data-struct/list_ul.h"
#include "result_types.h"

traversal_result*
dfs(in_memory_file_t* db,
    unsigned long     source_node_id,
    direction_t       direction,
    const char*       log_path)
{
    unsigned long* parents = malloc(db->node_id_counter * sizeof(*parents));
    unsigned long* dfs     = malloc(db->node_id_counter * sizeof(*dfs));

    if (!parents || !dfs) {
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        parents[i] = UNINITIALIZED_LONG;
        dfs[i]     = ULONG_MAX;
    }

    list_ul_t* node_stack = create_list_ul();
    FILE*      log_file   = fopen(log_path, "w");

    if (log_file == NULL) {
        free(parents);
        free(dfs);
        list_ul_destroy(node_stack);
        printf("bfs: Failed to open log file, %d\n", errno);
        return NULL;
    }

    list_relationship_t* current_rels = NULL;
    relationship_t*      current_rel  = NULL;
    unsigned long        temp;
    unsigned long*       node_id;
    list_ul_append(node_stack, source_node_id);
    dfs[source_node_id] = 0;

    size_t stack_size = list_ul_size(node_stack);

    while (stack_size > 0) {
        for (size_t j = 0; j < list_ul_size(node_stack); ++j) {
        }

        node_id      = list_ul_take(node_stack, stack_size - 1);
        current_rels = in_memory_expand(db, *node_id, direction);
        fprintf(log_file, "%s %lu\n", "dfs: Node: ", *node_id);

        for (size_t i = 0; i < list_relationship_size(current_rels); ++i) {
            current_rel = list_relationship_get(current_rels, i);
            fprintf(
                  log_file, "%s %lu\n", "dfs: Relationship: ", current_rel->id);
            temp = *node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (dfs[temp] == ULONG_MAX) {
                dfs[temp]     = dfs[*node_id] + 1;
                parents[temp] = current_rel->id;
                list_ul_append(node_stack, temp);
            }
        }
        stack_size = list_ul_size(node_stack);
        list_relationship_destroy(current_rels);
        free(node_id);
    }
    list_ul_destroy(node_stack);
    fclose(log_file);

    return create_traversal_result(source_node_id, dfs, parents);
}
