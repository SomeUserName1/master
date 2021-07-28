#include "query/dfs.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/linked_list.h"
#include "query/result_types.h"

traversal_result*
dfs(heap_file*    hf,
    unsigned long source_node_id,
    direction_t   direction,
    const char*   log_path)
{
    unsigned long* parents = malloc(hf->n_nodes * sizeof(*parents));
    unsigned long* dfs     = malloc(hf->n_nodes * sizeof(*dfs));

    if (!parents || !dfs) {
        printf("dfs: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        parents[i] = UNINITIALIZED_LONG;
        dfs[i]     = ULONG_MAX;
    }

    stack_ul* node_stack = st_ul_create();
    FILE*     log_file   = fopen(log_path, "w+");

    if (log_file == NULL) {
        free(parents);
        free(dfs);
        stack_ul_destroy(node_stack);
        printf("bfs: Failed to open log file, %d\n", errno);
        return NULL;
    }

    array_list_relationship* current_rels = NULL;
    relationship_t*          current_rel  = NULL;
    unsigned long            temp;
    unsigned long            node_id;
    stack_ul_push(node_stack, source_node_id);
    dfs[source_node_id] = 0;

    size_t stack_size = stack_ul_size(node_stack);

    while (stack_size > 0) {
        node_id      = stack_ul_pop(node_stack);
        current_rels = expand(hf, node_id, direction);
        fprintf(log_file, "%s %lu\n", "N", node_id);

        for (size_t i = 0; i < array_list_relationship_size(current_rels);
             ++i) {
            current_rel = array_list_relationship_get(current_rels, i);
            fprintf(log_file, "%s %lu\n", "R", current_rel->id);
            temp = node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (dfs[temp] == ULONG_MAX) {
                dfs[temp]     = dfs[node_id] + 1;
                parents[temp] = current_rel->id;
                stack_ul_push(node_stack, temp);
            }
        }
        stack_size = stack_ul_size(node_stack);
        array_list_relationship_destroy(current_rels);
    }
    stack_ul_destroy(node_stack);
    fclose(log_file);

    return create_traversal_result(source_node_id, dfs, parents);
}
