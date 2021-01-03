#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stddef.h>
#include <stdbool.h>


typedef bool (*queue_eq)(const void* a, const void* b);
typedef void* (*queue_copy)(const void* original);
typedef void (*queue_free)(void* elem);

typedef struct {
    queue_eq qeq;
    queue_copy qcopy;
    queue_free qfree;
} queue_cbs_t;

typedef struct queue_node {
    void* element;
    struct queue_node* prev;
    struct queue_node* next;
}   queue_node_t;

typedef struct queue {
    queue_node_t* head;
    queue_node_t* tail;
    size_t len;
    queue_cbs_t cbs;

    void (*destroy)(struct queue* queue);
    size_t (*size)(struct queue* queue);
    int (*add)(struct queue* queue, void* elem);
    int (*insert)(struct queue* queue, void* elem, size_t idx);
    int (*remove)(struct queue*, size_t idx);
    int (*remove_elem)(struct queue* queue, void* elem);
    int (*index_of)(struct queue* queue, void* elem, size_t* idx);
    bool (*contains)(struct queue* queue, void* elem);
    void* (*get)(struct queue* queue, size_t idx);
    void* (*take)(struct queue* queue);
} queue_t;

queue_t* create_queue(const queue_cbs_t* cbs);
void queue_destroy(queue_t* queue);
size_t queue_size(queue_t* queue);

int queue_add(queue_t* queue, void* elem);
int queue_insert(queue_t* queue, void* elem, size_t idx);
int queue_remove(queue_t* queue, size_t idx);
int queue_remove_elem(queue_t* queue, void* elem);

int queue_index_of(queue_t* queue, void* elem, size_t* idx);
bool queue_contains(queue_t* queue, void* elem);

void* queue_get(queue_t* queue, size_t idx);
void* queue_take(queue_t* queue);

#endif
