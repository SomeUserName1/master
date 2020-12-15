#include "../storage/storage.h"

list_t* bfs(node_t* source, node_t* target) {
    dict_t* parents = create_dict();
    dict_t* bfs = create_dict();
    queue_t* nodes_queue = create_queue();
    dict_t* visited_rels = create_dict();

    relationship_t* current_rel;
    node_t* current_node;
    nodes_queue->enqueue(source);
    bfs->add(source, 0);
    
    while (!nodes_queue->empty()) {
        current_node = get_node(nodes_queue->dequeue());
        current_rel = get_relationship(current_node->first_relationship);

        while (current_rel->id != UNINITIALIZED_LONG) {
            if (!parents->contains_key(current_rel->target_node)) {
                bfs->add(current_rel->target_node, bfs->get(current_node) + 1);
                parents->add(current_rel->target_node, current_rel);
                nodes_queue->enqueue(current_rel->target_node);
                current_rel = get_relationship(current_rel->next_rel_source);
               
                if (current_rel->target_node == target->id) {
                    return construct_path(source, target, parents);
               }
            }
        }
    }
    return create_list();
}


list_t* create_path(node_t* source, node_t* target, dict_t* parents) {
   list_t* result = create_list();

   node_t* child = target;
   relationship_t* rel;
   while(child != source) {
       rel = parents->get(child);
        result->add(rel);
        child = rel->source_node;
   }

   return result;
}
