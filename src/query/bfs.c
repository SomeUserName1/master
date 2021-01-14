#include "../record/node.h"
#include "../record/relationship.h"
#include "../data-struct/queue_ul.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/list.h"
#include "../data-struct/list_ul.h"
#include "../access/in_memory_file.h"
#include <stdio.h>
#include <errno.h>


// FIXME: Correct line 80, 22

list_ul_t* construct_path(in_memory_file_t* db, unsigned long source, unsigned long target, dict_ul_ul_t* parents, FILE* log_file) {
   list_ul_t* result = create_list_ul(LIST_NONE);

   unsigned long child = target;
   relationship_t* rel;
   while(child != source) {
        rel = in_memory_get_relationship(db, dict_ul_ul_get_direct(parents, child));
        fprintf(log_file, "%s %lu\n", "Relationship: ", rel->id);
        list_ul_append(result, rel->id);
        child = rel->source_node;
   }

   return result;
}

list_ul_t* bfs(in_memory_file_t* db, unsigned long sourceNodeID, unsigned long targetNodeID, const char* log_path) {
    dict_ul_ul_t* parents = create_dict_ul_ul();
    dict_ul_int_t* bfs = create_dict_ul_int();
    queue_ul_t* nodes_queue = create_queue_ul();
    FILE* log_file = fopen(log_path, "w");
    //dict_ul_ul_t* visited_rels = create_dict_ul_ul();

    if (log_file == NULL) {
        dict_ul_ul_destroy(parents);
        dict_ul_int_destroy(bfs);
        queue_ul_destroy(nodes_queue);
        printf("Failed to open log file, %d\n", errno);
        return create_list_ul(LIST_NONE);
    }

    relationship_t* current_rel = NULL;
    node_t* current_node = NULL;
    unsigned long temp;
    unsigned long node_id;
    unsigned long rel_id;
    if (queue_ul_add(nodes_queue, sourceNodeID < 0)) {
        dict_ul_ul_destroy(parents);
        dict_ul_int_destroy(bfs);
        queue_ul_destroy(nodes_queue);
        fclose(log_file);
        printf("Failed to insert node!\n");
        return NULL;
    }
    if (dict_ul_int_insert(bfs, sourceNodeID, 0) < 0) {
        printf("failed to insert bfs number for node\n");
    }

    while (queue_ul_size(nodes_queue) > 0) {
        node_id = queue_ul_take(nodes_queue);
        current_node = in_memory_get_node(db, node_id);
        fprintf(log_file, "%s %lu\n", "Node: ", node_id);

        if (current_node->id == targetNodeID) {
            list_ul_t* result = construct_path(db, sourceNodeID, targetNodeID, parents, log_file);
            dict_ul_ul_destroy(parents);
            dict_ul_int_destroy(bfs);
            queue_ul_destroy(nodes_queue);
            fclose(log_file);
            return result;
         }

        rel_id = current_node->first_relationship;
        while (rel_id != UNINITIALIZED_LONG ) {
            //&& !dict_ul_ul_contains(visited_rels, rel_id)
            // dict_ul_ul_insert(visited_rels, rel_id, 1);
            current_rel = in_memory_get_relationship(db, rel_id);
            fprintf(log_file, "%s %lu\n", "Relationship: ", rel_id);
            temp = current_rel->target_node;
            if (!dict_ul_ul_contains(parents, temp)) {
                dict_ul_int_insert(bfs, temp, dict_ul_int_get_direct(bfs, current_node->id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                queue_ul_add(nodes_queue, temp);
            }
            rel_id = current_node->id == current_rel->source_node ? current_rel->next_rel_source : current_rel->next_rel_target;
        }
    }
    dict_ul_ul_destroy(parents);
    dict_ul_int_destroy(bfs);
    queue_ul_destroy(nodes_queue);
    fclose(log_file);

    return create_list_ul(LIST_NONE);
}

