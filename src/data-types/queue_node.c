#include "queue_node.h"
#include "../storage/records/node.h"
#include "queue.h"

static bool node_eq(const void* first, const void* second) {
    return node_equals((node_t*) first, (node_t*) second);
}

static void* n_copy(const void* original) {
    return (void*) node_copy((node_t*) original);
}

queue_node_t* create_queue_node(void) {
    queue_cbs_t cbs = {
        node_eq,
        n_copy,
        free
    };

    return (queue_node_t*) create_queue(&cbs);
}
