#include "query/dfs.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "access/relationship.h"
#include "constants.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "query/result_types.h"

traversal_result*
dfs(heap_file*    hf,
    unsigned long source_node_id,
    direction_t   direction,
    const char*   log_path)
{
    if (!hf || source_node_id == UNINITIALIZED_LONG) {
        printf("dfs: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    dict_ul_ul* parents = d_ul_ul_create();
    dict_ul_ul* dfs     = d_ul_ul_create();

    stack_ul* node_stack = st_ul_create();
    FILE*     log_file   = fopen(log_path, "w+");

    if (log_file == NULL) {
        dict_ul_ul_destroy(parents);
        dict_ul_ul_destroy(dfs);
        stack_ul_destroy(node_stack);
        printf("bfs: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }

    array_list_relationship* current_rels = NULL;
    relationship_t*          current_rel  = NULL;
    unsigned long            temp;
    unsigned long            node_id;
    stack_ul_push(node_stack, source_node_id);
    dict_ul_ul_insert(dfs, source_node_id, 0);

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

            if (!dict_ul_ul_contains(dfs, temp)) {
                dict_ul_ul_insert(
                      dfs, temp, dict_ul_ul_get_direct(dfs, node_id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
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
