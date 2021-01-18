#include "bfs.h"
#include "../record/node.h"
#include "../record/relationship.h"
#include "../data-struct/queue_ul.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/set_ul.h"
#include "../access/in_memory_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

bfs_result_t* create_bfs_result(dict_ul_int_t* bfs, dict_ul_ul_t* parents) {
    bfs_result_t* result = malloc(sizeof(*result));
    result->bfs = bfs;
    result->parents = parents;

    return result;
}

void bfs_result_destroy(bfs_result_t* result) {
    dict_ul_int_destroy(result->bfs);
    dict_ul_ul_destroy(result->parents);
    free(result);
}

bfs_result_t* bfs(in_memory_file_t* db, unsigned long sourceNodeID, const char* log_path) {
    dict_ul_ul_t* parents = create_dict_ul_ul();
    dict_ul_int_t* bfs = create_dict_ul_int();
    queue_ul_t* nodes_queue = create_queue_ul();
    FILE* log_file = fopen(log_path, "w");

    if (log_file == NULL) {
        dict_ul_ul_destroy(parents);
        queue_ul_destroy(nodes_queue);
        printf("bfs: Failed to open log file, %d\n", errno);
        return NULL;
    }

    relationship_t* current_rel = NULL;
    node_t* current_node = NULL;
    unsigned long temp;
    unsigned long* node_id;
    unsigned long rel_id;
    queue_ul_add(nodes_queue, sourceNodeID);
    dict_ul_int_insert(bfs, sourceNodeID, 0);

    while (queue_ul_size(nodes_queue) > 0) {
        node_id = queue_ul_take(nodes_queue);
        current_node = in_memory_get_node(db, *node_id);
        fprintf(log_file, "%s %lu\n", "bfs: Node: ", *node_id);

        rel_id = current_node->first_relationship;
        while (rel_id != UNINITIALIZED_LONG) {
            current_rel = in_memory_get_relationship(db, rel_id);
            fprintf(log_file, "%s %lu\n", "bfs: Relationship: ", rel_id);
            temp = *node_id == current_rel->source_node ? current_rel->target_node : current_rel->source_node;

            if (!dict_ul_int_contains(bfs, temp)) {
                dict_ul_int_insert(bfs, temp, dict_ul_int_get_direct(bfs, *node_id));
                dict_ul_ul_insert(parents, temp, current_rel->id);
                queue_ul_add(nodes_queue, temp);
            }
            rel_id = in_memory_next_relationship(db, current_node->id, current_rel, BOTH);
        }
        free(node_id);
    }
    queue_ul_destroy(nodes_queue);
    fclose(log_file);

    return create_bfs_result(bfs, parents);
}

