#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

static const size_t list_block_size = 4;

typedef bool (*list_eq)(const void* a, const void* b);
typedef void* (*list_copy_cb)(const void*);
typedef void (*list_free_cb)(void*);

typedef struct
{
    list_eq leq;
    list_copy_cb lcopy;
    list_free_cb lfree;
} list_cbs_t;

typedef struct list
{
    void** elements;
    size_t alloced;
    size_t len;
    list_cbs_t cbs;
    bool inbulk;
} list_t;

list_t*
create_list(list_cbs_t* cbs);
void
list_destroy(list_t* l);
size_t
list_size(list_t* l);

int
list_append(list_t* l, void* v);
int
list_insert(list_t* l, void* v, size_t idx);
int
list_remove(list_t* l, size_t idx);
int
list_remove_elem(list_t* l, void* elem);

int
list_index_of(list_t* l, void* v, size_t* idx);
bool
list_contains(list_t* l, void* v);

void*
list_get(list_t* l, size_t idx);
void*
list_take(list_t* l, size_t idx);

#endif
