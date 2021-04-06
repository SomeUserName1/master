#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool
list_passthrough_eq(const void* first, const void* second)
{
    return first == second;
}

list_t*
create_list(list_cbs_t* cbs)
{
    if (!cbs) {
        printf("list - create: Invalid Arguments!\n");
        exit(-1);
    }

    list_t* list = malloc(sizeof(*list));

    if (!list) {
        exit(-1);
    }

    list->elements = malloc(sizeof(*list->elements) * list_block_size);
    if (!list->elements) {
        exit(-1);
    }

    list->alloced = list_block_size;
    list->len     = 0;

    memset(&(list->cbs), 0, sizeof(list->cbs));
    if (cbs != NULL) {
        list->cbs.leq   = cbs->leq;
        list->cbs.lcopy = cbs->lcopy;
        list->cbs.lfree = cbs->lfree;
    }

    if (list->cbs.leq == NULL) {
        list->cbs.leq = list_passthrough_eq;
    }

    return list;
}

void
list_destroy(list_t* l)
{
    if (l == NULL) {
        return;
    }

    if (l->cbs.lfree != NULL) {
        for (size_t i = 0; i < l->len; ++i) {
            l->cbs.lfree(l->elements[i]);
        }
    }

    free(l->elements);
    free(l);
}

size_t
list_size(list_t* l)
{
    if (l == NULL) {
        return 0;
    }
    return l->len;
}

int
list_append(list_t* l, void* v)
{
    if (l == NULL || v == NULL) {
        printf("list - append: Invalid Arguments!\n");
        exit(-1);
    }
    return list_insert(l, v, l->len);
}

int
list_insert(list_t* l, void* v, size_t idx)
{
    if (l == NULL || v == NULL) {
        printf("list - insert: Invalid Arguments!\n");
        exit(-1);
    }

    if (l->alloced == l->len) {
        l->alloced += list_block_size;
        l->elements = realloc(l->elements, sizeof(*l->elements) * l->alloced);
        if (!l->elements) {
            printf("list - insert: Memory Allocation Failed!\n");
            exit(-1);
        }
    }

    if (l->cbs.lcopy != NULL) {
        v = l->cbs.lcopy(v);
        if (v == NULL) {
            printf("list - insert: Copying the value failed!\n");
            exit(-1);
        }
    }

    if (idx > l->len) {
        idx = l->len;
    } else if (idx < l->len) {
        memmove(l->elements + idx + 1,
                l->elements + idx,
                (l->len - idx) * sizeof(*l->elements));
    }
    l->elements[idx] = v;
    l->len++;

    return 0;
}

static int
list_remove_int(list_t* l, size_t idx, bool free_flag)
{
    if (l == NULL || idx >= l->len) {
        printf("list - remove or take: Invalid Arguments!\n");
        exit(-1);
    }

    if (free_flag && l->cbs.lfree != NULL) {
        l->cbs.lfree(l->elements[idx]);
    }

    l->len--;
    if (idx == l->len) {
        return 0;
    }
    memmove(l->elements + idx,
            l->elements + idx + 1,
            (l->len - idx) * sizeof(*l->elements));
    return 0;
}

int
list_remove(list_t* l, size_t idx)
{
    return list_remove_int(l, idx, true);
}

int
list_index_of(list_t* l, void* v, size_t* idx)
{
    if (l == NULL || v == NULL || idx == NULL) {
        printf("list - index_of: Invalid Arguments!\n");
        exit(-1);
    }

    for (size_t i = 0; i < l->len; ++i) {
        if (l->cbs.leq(v, l->elements[i])) {
            *idx = i;
            return 0;
        }
    }
    return -1;
}

int
list_remove_elem(list_t* l, void* elem)
{
    size_t idx = 0;
    if (list_index_of(l, elem, &idx) != 0) {
        return -1;
    }
    return list_remove_int(l, idx, true);
}

bool
list_contains(list_t* l, void* elem)
{
    size_t idx = 0;
    return list_index_of(l, elem, &idx) == 0 ? true : false;
}

void*
list_get(list_t* l, size_t idx)
{
    if (!l) {
        printf("list - index_of: Invalid Arguments: List must not be NULL!\n");
        exit(-1);
    }
    if (idx >= l->len) {
        printf("List - get: Buffer overflow! list length %lu, index accessed "
               "%lu \n",
               l->len,
               idx);
        exit(-1);
    }
    return l->elements[idx];
}

void*
list_take(list_t* l, size_t idx)
{
    void* elem = list_get(l, idx);
    list_remove_int(l, idx, false);
    return elem;
}
