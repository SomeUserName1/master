#include "../storage/records/node.h"
#include "../storage/records/relationship.h"
#include "../data-types/queue_ul.h"
#include "../data-types/dict_ul.h"
#include "../data-types/list_ul.h"

list_ul_t* create_path(unsigned long source, unsigned long target, dict_ul_ul_t* parents) {
   list_ul_t* result = create_list_ul(LIST_NONE);

   unsigned long child = target;
   relationship_t* rel;
   while(child != source) {
        rel = get_relationship(dict_ul_ul_get_direct(parents, child));
        list_ul_append(result, rel->id);
        child = rel->source_node;
   }

   return result;
}

list_ul_t* bfs(unsigned long sourceNodeID, unsigned long targetNodeID) {
    dict_ul_ul_t* parents = create_dict_ul_ul();
    dict_ul_int_t* bfs = create_dict_ul_int();
    queue_ul_t* nodes_queue = create_queue_node();

    relationship_t* current_rel;
    node_t* current_node;
    unsigned long temp;
    queue_ul_add(nodes_queue, sourceNodeID);
    dict_ul_int_insert(bfs, sourceNodeID, 0);
    
    while (queue_ul_size(nodes_queue) > 0) {
        current_node = get_node(queue_ul_take(nodes_queue));
        current_rel = get_relationship(current_node->first_relationship);

        while (current_rel->id != UNINITIALIZED_LONG) {
            temp = current_rel->target_node;
            if (!dict_ul_ul_contains(parents, temp)) {
                dict_ul_int_insert(bfs, temp, dict_ul_int_get_direct(bfs, current_node->id) + 1);
                dict_ul_ul_insert(parents, temp, current_rel->id);
                queue_ul_add(nodes_queue, temp);
                current_rel = get_relationship(current_rel->next_rel_source);
               
                if (current_rel->target_node == targetNodeID) {
                    list_ul_t* result = construct_path(sourceNodeID, targetNodeID, parents);
                    dict_ul_ul_destroy(parents);
                    dict_ul_int_destroy(bfs);
                    queue_ul_destroy(nodes_queue);
                    return result;
               }
            }
        }
    }
    return create_list_ul(LIST_NONE);
}



