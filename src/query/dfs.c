#include "dfs.h"

#include <errno.h>
#include <stdio.h>

#include "../data-struct/list_ul.h"
#include "result_types.h"

search_result_t*
dfs(in_memory_file_t* db,
    unsigned long source_node_id,
    direction_t direction,
    const char* log_path)
{
    dict_ul_ul_t* parents = create_dict_ul_ul();
    dict_ul_int_t* dfs = create_dict_ul_int();
    list_ul_t* node_stack = create_list_ul();
    FILE* log_file = fopen(log_path, "w");

    if (log_file == NULL) {
        dict_ul_ul_destroy(parents);
        dict_ul_int_destroy(dfs);
        list_ul_destroy(node_stack);
        printf("bfs: Failed to open log file, %d\n", errno);
        return NULL;
    }

    list_relationship_t* current_rels = NULL;
    relationship_t* current_rel = NULL;
    unsigned long temp;
    unsigned long node_id;
    list_ul_append(node_stack, source_node_id);
    dict_ul_int_insert(dfs, source_node_id, 0);

    size_t stack_size = list_ul_size(node_stack);

    while (stack_size > 0) {
        node_id = list_ul_take(node_stack, stack_size - 1);
        current_rels = in_memory_expand(db, node_id, direction);
        fprintf(log_file, "%s %lu\n", "bfs: Node: ", node_id);

        for (size_t i = 0; i < list_relationship_size(current_rels); ++i) {
            current_rel = list_relationship_get(current_rels, i);
            fprintf(
                  log_file, "%s %lu\n", "bfs: Relationship: ", current_rel->id);
            temp = node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (!dict_ul_int_contains(dfs, temp)) {
                dict_ul_int_insert(
                      dfs, temp, dict_ul_int_get_direct(dfs, node_id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                list_ul_append(node_stack, temp);
            }
        }
        stack_size = list_ul_size(node_stack);
        list_relationship_destroy(current_rels);
    }
    list_ul_destroy(node_stack);
    fclose(log_file);

    return create_search_result(dfs, parents);
}
