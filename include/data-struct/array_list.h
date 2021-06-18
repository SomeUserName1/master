#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LIST_CBS_TYPEDEF(typename, T)                                    \
    typedef bool (*typename##_eq)(const T, const T);                           \
    typedef T (*typename##_copy_cb)(const T*);                                 \
    typedef void (*typename##_free_cb)(T*);

#define ARRAY_LIST_STRUCTS(typename, T)                                        \
    static const size_t initial_alloc = 128;                                   \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename##_eq      leq;                                                \
        typename##_copy_cb lcopy;                                              \
        typename##_free_cb lfree;                                              \
    } typename##_cbs;                                                          \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        T*             elements;                                               \
        size_t         alloced;                                                \
        size_t         len;                                                    \
        typename##_cbs cbs;                                                    \
    } typename;

#define ARRAY_LIST_PASSTHROUGH_CBS(typename, T)                                \
    static bool typename##_passthrough_eq(const T first, const T second)       \
    {                                                                          \
        return first == second;                                                \
    }

#define ARRAY_LIST_CREATE(typename, T)                                         \
    typename* typename##_create(typename##_cbs cbs)                            \
    {                                                                          \
        typename* list = calloc(sizeof(*list));                                \
                                                                               \
        if (!list) {                                                           \
            printf("Failed to allocate structs for array list!\n");            \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        list->elements = calloc(sizeof(*list->elements) * initial_alloc);      \
        if (!list->elements) {                                                 \
            printf("Failed to allocate data array for array list!\n");         \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        list->alloced = initial_alloc;                                         \
        list->len     = 0;                                                     \
                                                                               \
        list->cbs.leq   = cbs.leq;                                             \
        list->cbs.lcopy = cbs.lcopy;                                           \
        list->cbs.lfree = cbs.lfree;                                           \
                                                                               \
        if (!list->cbs.leq) {                                                  \
            list->cbs.leq = typename##_passthrough_eq;                         \
        }                                                                      \
                                                                               \
        return list;                                                           \
    }

#define ARRAY_LIST_DESTROY(typename)                                           \
    void typename##_destroy(typename* l)                                       \
    {                                                                          \
        if (!l) {                                                              \
            return;                                                            \
        }                                                                      \
                                                                               \
        if (l->cbs.lfree) {                                                    \
            for (size_t i = 0; i < l->len; ++i) {                              \
                l->cbs.lfree(l->elements[i]);                                  \
            }                                                                  \
        }                                                                      \
                                                                               \
        free(l->elements);                                                     \
        free(l);                                                               \
    }

#define ARRAY_LIST_SIZE(typename)                                              \
    size_t typename##_size(typename* l)                                        \
    {                                                                          \
        if (!l) {                                                              \
            return 0;                                                          \
        }                                                                      \
        return l->len;                                                         \
    }

#define ARRAY_LIST_APPEND(typename, T)                                         \
    int typename##_append(typename* l, T v)                                    \
    {                                                                          \
        if (!l) {                                                              \
            printf("list - append: Invalid Arguments!\n");                     \
            exit(-1);                                                          \
        }                                                                      \
        return typename##_insert(l, v, l->len);                                \
    }

#define ARRAY_LIST_INSERT(typename, T)                                         \
    int typename##_insert(typename* l, T v, size_t idx)                        \
    {                                                                          \
        if (!l) {                                                              \
            printf("list - insert: Invalid Arguments!\n");                     \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        if (l->alloced == l->len) {                                            \
            l->alloced <<= 2;                                                  \
                                                                               \
            T* realloc_h =                                                     \
                  realloc(l->elements, sizeof(*l->elements) * l->alloced);     \
            if (!realloc_h) {                                                  \
                free(l->elements);                                             \
                printf("list - insert: Memory Allocation Failed!\n");          \
                exit(-1);                                                      \
            } else {                                                           \
                l->elements = realloc_h;                                       \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (l->cbs.lcopy) {                                                    \
            v = l->cbs.lcopy(v);                                               \
        }                                                                      \
                                                                               \
        if (idx > l->len) {                                                    \
            idx = l->len;                                                      \
        } else if (idx < l->len) {                                             \
            memmove(l->elements + idx + 1,                                     \
                    l->elements + idx,                                         \
                    (l->len - idx) * sizeof(*l->elements));                    \
        }                                                                      \
        l->elements[idx] = v;                                                  \
        l->len++;                                                              \
                                                                               \
        return 0;                                                              \
    }

#define ARRAY_LIST_REMOVE_INTERNAL(typename)                                   \
    static int                                                                 \
          typename##_remove_internal(typename* l, size_t idx, bool free_flag)  \
    {                                                                          \
        if (!l || idx >= l->len) {                                             \
            printf("list - remove or take: Invalid Arguments!\n");             \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        if (free_flag && l->cbs.lfree) {                                       \
            l->cbs.lfree(l->elements[idx]);                                    \
        }                                                                      \
                                                                               \
        l->len--;                                                              \
        if (idx == l->len) {                                                   \
            return 0;                                                          \
        }                                                                      \
        memmove(l->elements + idx,                                             \
                l->elements + idx + 1,                                         \
                (l->len - idx) * sizeof(*l->elements));                        \
        return 0;                                                              \
    }

#define ARRAY_LIST_REMOVE(typename)                                            \
    int typename##_remove(typename* l, size_t idx)                             \
    {                                                                          \
        return typename##_remove_internal(l, idx, true);                       \
    }

#define ARRAY_LIST_INDEX_OF(typename, T)                                       \
    int typename##_index_of(typename* l, T v, size_t* idx)                     \
    {                                                                          \
        if (!l || !idx) {                                                      \
            printf("list - index_of: Invalid Arguments!\n");                   \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        for (size_t i = 0; i < l->len; ++i) {                                  \
            if (l->cbs.leq(v, l->elements[i])) {                               \
                *idx = i;                                                      \
                return 0;                                                      \
            }                                                                  \
        }                                                                      \
        return -1;                                                             \
    }

#define ARRAY_LIST_REMOVE_ELEM(typename, T)                                    \
    int typename##_remove_elem(typename* l, T elem)                            \
    {                                                                          \
        size_t idx = 0;                                                        \
        if (typename##_index_of(l, elem, &idx) != 0) {                         \
            return -1;                                                         \
        }                                                                      \
        return typename##_remove_internal(l, idx, true);                       \
    }

#define ARRAY_LIST_CONTAINS(typename, T)                                       \
    bool typename##_contains(typename* l, T elem)                              \
    {                                                                          \
        size_t idx = 0;                                                        \
        return typename##_index_of(l, elem, &idx) == 0 ? true : false;         \
    }

#define ARRAY_LIST_GET(typename, T)                                            \
    T typename##_get(typename* l, size_t idx)                                  \
    {                                                                          \
        if (!l) {                                                              \
            printf("list - index_of: Invalid Arguments: List must not be "     \
                   "NULL!\n");                                                 \
            exit(-1);                                                          \
        }                                                                      \
        if (idx >= l->len) {                                                   \
            printf("List - get: Buffer overflow! list length %lu, index "      \
                   "accessed "                                                 \
                   "%lu \n",                                                   \
                   l->len,                                                     \
                   idx);                                                       \
            exit(-1);                                                          \
        }                                                                      \
        return l->elements[idx];                                               \
    }

#define ARRAY_LIST_TAKE(typename, T)                                           \
    T typename##_take(typename* l, size_t idx)                                 \
    {                                                                          \
        T elem = typename##_get(l, idx);                                       \
        typename##_remove_internal(l, idx, false);                             \
        return elem;                                                           \
    }

#endif
