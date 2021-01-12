#include "../record/node.h"
#include "../record/relationship.h"
#include "../data-struct/queue_ul.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/list.h"
#include "../data-struct/list_ul.h"
#include "../access/in_memory_file.h"
#include <stdio.h>

list_ul_t* construct_path(in_memory_file_t* db, unsigned long source, unsigned long target, dict_ul_ul_t* parents) {
   list_ul_t* result = create_list_ul(LIST_NONE);

   unsigned long child = target;
   relationship_t* rel;
   while(child != source) {
        rel = in_memory_get_relationship(db, dict_ul_ul_get_direct(parents, child));
        list_ul_append(result, rel->id);
        child = rel->source_node;
   }

   return result;
}

list_ul_t* bfs(in_memory_file_t* db, unsigned long sourceNodeID, unsigned long targetNodeID) {
    dict_ul_ul_t* parents = create_dict_ul_ul();
    dict_ul_int_t* bfs = create_dict_ul_int();
    queue_ul_t* nodes_queue = create_queue_ul();

    relationship_t* current_rel;
    node_t* current_node;
    unsigned long temp;
    unsigned long node_id;
    unsigned long rel_id;
    queue_ul_add(nodes_queue, sourceNodeID);
    dict_ul_int_insert(bfs, sourceNodeID, 0);

    while (queue_ul_size(nodes_queue) > 0) {
        node_id = queue_ul_take(nodes_queue);
        current_node = in_memory_get_node(db, node_id);
        printf("%s %lu\n", "Accessing node with ID: ", node_id);

        if (current_rel->target_node == targetNodeID) {
                    list_ul_t* result = construct_path(db, sourceNodeID, targetNodeID, parents);
                    dict_ul_ul_destroy(parents);
                    dict_ul_int_destroy(bfs);
                    queue_ul_destroy(nodes_queue);
                    return result;
         }

        rel_id = current_node->first_relationship;

        while (current_rel->id != UNINITIALIZED_LONG) {
            printf("%s %lu\n", "Accessing edge with ID: ", rel_id);
            current_rel = in_memory_get_relationship(db, rel_id);
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

    return create_list_ul(LIST_NONE);
}

