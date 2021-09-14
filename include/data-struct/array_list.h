/*
 * @(#)array_list.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */

#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LIST_DECL(typename, T)                                           \
    ARRAY_LIST_CBS_TYPEDEF(typename, T)                                        \
    ARRAY_LIST_STRUCTS(typename, T)                                            \
    typename* typename##_create(typename##_cbs cbs);                           \
    void typename##_destroy(typename* l);                                      \
    size_t typename##_size(typename* l);                                       \
    void typename##_insert(typename* l, T v, size_t idx);                      \
    void typename##_append(typename* l, T v);                                  \
    void typename##_remove(typename* l, size_t idx);                           \
    int typename##_index_of(typename* l, T v, size_t* idx);                    \
    void typename##_remove_elem(typename* l, T elem);                          \
    bool typename##_contains(typename* l, T elem);                             \
    T typename##_get(typename* l, size_t idx);                                 \
    T typename##_take(typename* l, size_t idx);

#define ARRAY_LIST_IMPL(typename, T)                                           \
    ARRAY_LIST_CREATE(typename, T)                                             \
    ARRAY_LIST_DESTROY(typename)                                               \
    ARRAY_LIST_SIZE(typename)                                                  \
    ARRAY_LIST_INSERT(typename, T)                                             \
    ARRAY_LIST_APPEND(typename, T)                                             \
    ARRAY_LIST_REMOVE_INTERNAL(typename, T)                                    \
    ARRAY_LIST_REMOVE(typename)                                                \
    ARRAY_LIST_INDEX_OF(typename, T)                                           \
    ARRAY_LIST_REMOVE_ELEM(typename, T)                                        \
    ARRAY_LIST_CONTAINS(typename, T)                                           \
    ARRAY_LIST_GET(typename, T)                                                \
    ARRAY_LIST_TAKE(typename, T)

static const size_t initial_alloc = 128;

#define ARRAY_LIST_CBS_TYPEDEF(typename, T)                                    \
    typedef bool (*typename##_eq)(const T, const T);                           \
    typedef T (*typename##_copy_cb)(const T);                                  \
    typedef void (*typename##_free_cb)(T);

#define ARRAY_LIST_STRUCTS(typename, T)                                        \
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

#define ARRAY_LIST_CREATE(typename, T)                                         \
    typename* typename##_create(typename##_cbs cbs)                            \
    {                                                                          \
        if (!cbs.leq) {                                                        \
            printf("array list - create: Invalid arguments! array list!\n");   \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename* list = calloc(1, sizeof(typename));                          \
                                                                               \
        if (!list) {                                                           \
            printf("Failed to allocate structs for array list!\n");            \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        list->elements = calloc(initial_alloc, sizeof(T));                     \
        if (!list->elements) {                                                 \
            printf("Failed to allocate data array for array list!\n");         \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        list->alloced = initial_alloc;                                         \
        list->len     = 0;                                                     \
                                                                               \
        list->cbs = cbs;                                                       \
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
    inline size_t typename##_size(typename* l)                                 \
    {                                                                          \
        if (!l) {                                                              \
            return 0;                                                          \
        }                                                                      \
        return l->len;                                                         \
    }

#define ARRAY_LIST_INSERT(typename, T)                                         \
    void typename##_insert(typename* l, T v, size_t idx)                       \
    {                                                                          \
        if (!l || idx < 0 || idx > l->len) {                                   \
            printf("list - insert: Invalid Arguments!\n");                     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        if (l->alloced == l->len) {                                            \
            l->alloced <<= 2;                                                  \
                                                                               \
            T* realloc_h = realloc(l->elements, sizeof(T) * l->alloced);       \
            if (!realloc_h) {                                                  \
                free(l->elements);                                             \
                printf("list - insert: Memory Allocation Failed!\n");          \
                exit(EXIT_FAILURE);                                            \
            } else {                                                           \
                l->elements = realloc_h;                                       \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (l->cbs.lcopy) {                                                    \
            v = l->cbs.lcopy(v);                                               \
        }                                                                      \
                                                                               \
        if (idx < l->len) {                                                    \
            memmove(l->elements + idx + 1,                                     \
                    l->elements + idx,                                         \
                    (l->len - idx) * sizeof(T));                               \
        }                                                                      \
        l->elements[idx] = v;                                                  \
        l->len++;                                                              \
    }

#define ARRAY_LIST_APPEND(typename, T)                                         \
    inline void typename##_append(typename* l, T v)                            \
    {                                                                          \
        if (!l) {                                                              \
            printf("list - append: Invalid Arguments!\n");                     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename##_insert(l, v, l->len);                                       \
    }

#define ARRAY_LIST_REMOVE_INTERNAL(typename, T)                                \
    static void                                                                \
          typename##_remove_internal(typename* l, size_t idx, bool free_flag)  \
    {                                                                          \
        if (!l || idx >= l->len) {                                             \
            printf("list - remove or take: Invalid Arguments!\n");             \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        if (free_flag && l->cbs.lfree) {                                       \
            l->cbs.lfree(l->elements[idx]);                                    \
        }                                                                      \
                                                                               \
        l->len--;                                                              \
        if (idx < l->len) {                                                    \
            memmove(l->elements + idx,                                         \
                    l->elements + idx + 1,                                     \
                    (l->len - idx) * sizeof(T));                               \
        }                                                                      \
    }

#define ARRAY_LIST_REMOVE(typename)                                            \
    inline void typename##_remove(typename* l, size_t idx)                     \
    {                                                                          \
        typename##_remove_internal(l, idx, true);                              \
    }

#define ARRAY_LIST_INDEX_OF(typename, T)                                       \
    int typename##_index_of(typename* l, T v, size_t* idx)                     \
    {                                                                          \
        if (!l || !idx) {                                                      \
            printf("list - index_of: Invalid Arguments!\n");                   \
            exit(EXIT_FAILURE);                                                \
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
    inline void typename##_remove_elem(typename* l, T elem)                    \
    {                                                                          \
        size_t idx = 0;                                                        \
        if (typename##_index_of(l, elem, &idx) != 0) {                         \
            printf("array list - remove element: Element not in list!\n");     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename##_remove_internal(l, idx, true);                              \
    }

#define ARRAY_LIST_CONTAINS(typename, T)                                       \
    inline bool typename##_contains(typename* l, T elem)                       \
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
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        if (idx >= l->len) {                                                   \
            printf("List - get: Buffer overflow! list length %lu, index "      \
                   "accessed "                                                 \
                   "%lu \n",                                                   \
                   l->len,                                                     \
                   idx);                                                       \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        return l->elements[idx];                                               \
    }

#define ARRAY_LIST_TAKE(typename, T)                                           \
    inline T typename##_take(typename* l, size_t idx)                          \
    {                                                                          \
        T elem = typename##_get(l, idx);                                       \
        typename##_remove_internal(l, idx, false);                             \
        return elem;                                                           \
    }

ARRAY_LIST_DECL(array_list_ul, unsigned long);

array_list_ul*
al_ul_create(void);

ARRAY_LIST_DECL(array_list_l, long);

array_list_l*
al_l_create(void);

#endif
