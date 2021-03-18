#include "bfs.h"
#include "../access/in_memory_file.h"
#include "../constants.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/queue_ul.h"
#include "../record/node.h"
#include "../record/relationship.h"
#include "result_types.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

search_result_t*
bfs(in_memory_file_t* db,
    unsigned long source_node_id,
    direction_t direction,
    const char* log_path)
{
    dict_ul_ul_t* parents = create_dict_ul_ul();
    dict_ul_int_t* bfs = create_dict_ul_int();
    queue_ul_t* nodes_queue = create_queue_ul();
    FILE* log_file = fopen(log_path, "w");

    if (log_file == NULL) {
        dict_ul_ul_destroy(parents);
        dict_ul_int_destroy(bfs);
        queue_ul_destroy(nodes_queue);
        printf("bfs: Failed to open log file, %d\n", errno);
        return NULL;
    }

    list_relationship_t* current_rels = NULL;
    relationship_t* current_rel = NULL;
    unsigned long temp;
    unsigned long* node_id;
    queue_ul_add(nodes_queue, source_node_id);
    dict_ul_int_insert(bfs, source_node_id, 0);

    while (queue_ul_size(nodes_queue) > 0) {
        node_id = queue_ul_take(nodes_queue);
        current_rels = in_memory_expand(db, *node_id, direction);
        fprintf(log_file, "%s %lu\n", "bfs: Node: ", *node_id);

        for (size_t i = 0; i < list_relationship_size(current_rels); ++i) {
            current_rel = list_relationship_get(current_rels, i);
            fprintf(
                  log_file, "%s %lu\n", "bfs: Relationship: ", current_rel->id);
            temp = *node_id == current_rel->source_node
                         ? current_rel->target_node
                         : current_rel->source_node;

            if (!dict_ul_int_contains(bfs, temp)) {
                dict_ul_int_insert(
                      bfs, temp, dict_ul_int_get_direct(bfs, *node_id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                queue_ul_add(nodes_queue, temp);
            }
        }
        free(node_id);
        list_relationship_destroy(current_rels);
    }
    queue_ul_destroy(nodes_queue);
    fclose(log_file);

    return create_search_result(bfs, parents);
}
