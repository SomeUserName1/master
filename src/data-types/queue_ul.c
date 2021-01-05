#include "queue.h"
#include "queue_ul.h"
#include "cbs.h"

#include <stdlib.h>

queue_ul_t* create_queue_node(void) {
    queue_cbs_t cbs = {
        unsigned_long_eq,
        unsigned_long_copy,
        free
    };

    return (queue_ul_t*) create_queue(&cbs);
}

void queue_ul_destroy(queue_ul_t* queue) {
    queue_destroy((queue_t*) queue);
}

size_t queue_ul_size(queue_ul_t* queue) {
    return queue_size((queue_t*) queue);
}

int queue_ul_add(queue_ul_t* queue, unsigned long elem) {
    return queue_add((queue_t*) queue, (void*) &elem);
}

int queue_ul_insert(queue_ul_t* queue, unsigned long elem, size_t idx) {
    return queue_insert((queue_t*) queue, (void*) &elem, idx);
}

int queue_ul_remove(queue_ul_t* queue, size_t idx) {
    return queue_remove((queue_t*) queue, idx);
}

int queue_ul_remove_elem(queue_ul_t* queue, unsigned long elem) {
    return queue_remove_elem((queue_t*) queue, (void*) &elem);
}

int queue_ul_index_of(queue_ul_t* queue, unsigned long elem, size_t* idx) {
    return queue_index_of((queue_t*) queue, (void*) &elem, idx);
}

bool queue_ul_contains(queue_ul_t* queue, unsigned long elem) {
    return queue_contains((queue_t*) queue, (void*) &elem);
}

unsigned long queue_ul_get(queue_ul_t* queue, size_t idx) {
    return *((unsigned long*) queue_get((queue_t*) queue, idx));
}

unsigned long queue_ul_take(queue_ul_t* queue) {
    return *((unsigned long*) queue_take((queue_t*) queue));
}
