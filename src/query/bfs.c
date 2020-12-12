#include "../storage/storage.h"

relationship_t* bfs(node_t* source, node_t* target) {
    relationship_t* first = source->first_relationship;

    if (first->target_node == target->id) {
    return first;    
    }

    
}
