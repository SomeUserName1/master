#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

static const size_t list_block_size = 4;

typedef bool (*list_eq)(const void* a, const void* b);
typedef void* (*list_copy_cb)(const void*);
typedef void (*list_free_cb)(void*);

typedef struct {
    list_eq      leq;
    list_copy_cb lcopy;
    list_free_cb lfree;
} list_cbs_t;

typedef enum {
    LIST_NONE = 0,
    LIST_SORT = 1 << 0
} list_flags_t;

    typedef struct list {
    void**          elements;
    size_t          alloced;
    size_t          len;
    list_cbs_t      cbs;
    list_flags_t    flags;
    bool            inbulk;
} list_t;

list_t* create_list(list_cbs_t* cbs, list_flags_t flags);
void list_destroy(list_t* l);
size_t list_size(list_t* l);

void* binary_search(const void* base, size_t nel, size_t width,
        const void* key, size_t* idx, bool (*cmp)(const void*, const void*));
int binary_insert(const void* base, size_t nel, size_t width,
        const void* key, size_t* idx, bool (*cmp)(const void*, const void*));

int list_append(list_t* l, void *v);
int list_insert(list_t* l, void *v, size_t idx);
int list_remove(list_t* l, size_t idx);
int list_remove_elem(list_t* l, void* elem);

int list_index_of(list_t* l, void* v, size_t* idx);
bool list_contains(list_t* l, void* v);

void* list_get(list_t* l, size_t idx);
void* list_take(list_t* l, size_t idx);

int list_start_bulk_add(list_t* l);
int list_end_bulk_add(list_t* l);

int merge_sort(void* base, size_t nel, size_t width,
        bool (*cmp)(const void*, const void*));
int list_sort(list_t* l, list_eq e);

#endif
