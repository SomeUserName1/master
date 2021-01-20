#ifndef QUEUE_UL_H
#define QUEUE_UL_H

#include <stdbool.h>
#include <stddef.h>

typedef struct queue_ul queue_ul_t;

queue_ul_t* create_queue_ul(void);
void queue_ul_destroy(queue_ul_t* queue);
size_t queue_ul_size(queue_ul_t* queue);

int queue_ul_add(queue_ul_t* queue, unsigned long elem);
int queue_ul_insert(queue_ul_t* queue, unsigned long elem, size_t idx);
int queue_ul_remove(queue_ul_t* queue, size_t idx);
int queue_ul_remove_elem(queue_ul_t* queue, unsigned long elem);

int queue_ul_index_of(queue_ul_t* queue, unsigned long elem, size_t* idx);
bool queue_ul_contains(queue_ul_t* queue, unsigned long elem);

unsigned long queue_ul_get(queue_ul_t* queue, size_t idx);
unsigned long* queue_ul_take(queue_ul_t* queue);
#endif
