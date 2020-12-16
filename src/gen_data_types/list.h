#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const size_t list_block_size = 32;

typedef int (*list_eq)(const void* a, const void* b);
typedef void* (*list_copy_cb)(void*);
typedef void (*list_free_cb)(void*);

typedef struct {
    list_eq      leq;
    list_copy_cb lcopy;
    list_free_cb lfree;
} list_cbs_t;

    typedef struct list {
    void**          elements;
    size_t          alloced;
    size_t          len;
    list_cbs_t      cbs;
    bool            inbulk;

    void (*destroy)(struct list* self);
    size_t (*size)(struct list* self);
    int (*add)(struct list* self, void* elem);
    int (*insert)(struct list* self, void* elem, size_t idx);
    int (*remove)(struct list* self, size_t idx);
    int (*index_of)(struct list* self, void* elem, size_t* idx);
    int (*contains)(struct list* self, void* elem);
    int (*get)(struct list* self, size_t idx);
    int (*take)(struct list* self, size_t idx);
} list_t;

list_t *create_list(const list_cbs_t* cbs);
void list_destroy(list_t* l);
size_t list_size(list_t* l);

int list_add(list_t* l, void *v);
int list_insert(list_t* l, void *v, size_t idx);
int list_remove(list_t* l, size_t idx);

int list_index_of(list_t* l, void* v, size_t* idx);
int list_contains(list_t* l, void* v);

void* list_get(list_t* l, size_t idx);
void* list_take(list_t* l, size_t idx);

#endif /* __LIST_H__ */
