/*!
 * \file array_list.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief This header defines Macros to generate declarations (header) and
 * definitions (implementation) for array lists of different types.
 * At the very end of this file, the declarations for array lists with primitive
 * type are generated. These need only the equality and no copy or free call
 * backs.
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
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

/*!
 * Generate the necessary declarations for an array list named typename storing
 * elements of type T.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 *  Generate the definitions/implementation for an array list of type T with
 * type name typename.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 * Defines types for functions that are type specific and are to be supplied to
 * the generated struct. The equality function is obligatory, while copy and
 * free are optional.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
#define ARRAY_LIST_CBS_TYPEDEF(typename, T)                                    \
    typedef bool (*typename##_eq)(const T, const T);                           \
    typedef T (*typename##_copy_cb)(const T);                                  \
    typedef void (*typename##_free_cb)(T);

/*!
 * Generates the structs that are used for the array list with the appropriate
 * type \p T and type name \p typename supplied. The structs include one that
 * holds the call backs (equality, copy, free) and the array list itself. The
 * array list struct stores an array (a pointer) of elements of type T, a
 * counter for the number of allocated items, a counter for its current length
 * and an instance of the call back struct.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 * Constructor for an array list of type \p T with type name \p typename.
 * Allocates memory for the array list struct, allocates initially space for 128
 * elements and sets the length to 0. Also sets the call back struct which needs
 * to be provided as argument to the generated function.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 *  Destructor for an array list instance.
 *  Frees the space allocated for the elements and the array list struct. If the
 * free call back is provided also frees the elements themselves (e.g. when the
 * elements are of type node_t*).
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 */
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

/*!
 * Returns the size of the array list.
 * The size is the number of used elements (in contrast to the number of
 * allocated elements).
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 */
#define ARRAY_LIST_SIZE(typename)                                              \
    inline size_t typename##_size(typename* l)                                 \
    {                                                                          \
        if (!l) {                                                              \
            return 0;                                                          \
        }                                                                      \
        return l->len;                                                         \
    }

/*!
 * Inserts a new element v of type T at position idx of array list l.
 * If the number of allocated elements is not enough, the list reallocs the
 * double amount of space.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 *  Appends an element v to the back of the array list. Essentially calls insert
 * with idx = l->len.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
#define ARRAY_LIST_APPEND(typename, T)                                         \
    inline void typename##_append(typename* l, T v)                            \
    {                                                                          \
        if (!l) {                                                              \
            printf("list - append: Invalid Arguments!\n");                     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename##_insert(l, v, l->len);                                       \
    }

/*!
 * Do not use directly: Used to implement remove and take.
 * Removes the element at index idx from the list l. If the free flag is set
 * (remove) the element is freed. Otherwise it's not (take).
 * decrements the number of elements and closes the gap in the array using
 * memmove.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 * Removes the element at postition idx from the list l. Also frees the element
 * if the free callback is provided an appropriate function.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 */
#define ARRAY_LIST_REMOVE(typename)                                            \
    inline void typename##_remove(typename* l, size_t idx)                     \
    {                                                                          \
        typename##_remove_internal(l, idx, true);                              \
    }

/*!
 * Provided with an instance v of type T, and a pointer to a size_t variable,
 * the generated function returns 0 if the element is present and sets the
 * size_t pointer to the index of the element in the list. Otherwise returns -1.
 * O(n) operation.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 * Removes an element from the list without requiring the index of the element.
 * The generated function calls index of to fetch the element and then removes
 * the element using remove_internal.
 * O(n) + O(memmove) operation.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 * Checks if an element is contained in the list.
 * O(n) operation.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
#define ARRAY_LIST_CONTAINS(typename, T)                                       \
    inline bool typename##_contains(typename* l, T elem)                       \
    {                                                                          \
        size_t idx = 0;                                                        \
        return typename##_index_of(l, elem, &idx) == 0 ? true : false;         \
    }

/*!
 * Returns the element at position idx from the list.
 * The element and its memory management remain within the list.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
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

/*!
 * Returns the element at position idx from the list, removing it from the
 * latter. The element is taken out of the list and the memory management of the
 * pointed to element (if the type \p Tis not a primitive and the copy and free
 * callbacks are set up) is passed on to the callee.
 *
 * \param typename The type name of the array list, e.g. array_list_node.
 * \param T The type of the elements to store, e.g. node_t.
 */
#define ARRAY_LIST_TAKE(typename, T)                                           \
    inline T typename##_take(typename* l, size_t idx)                          \
    {                                                                          \
        T elem = typename##_get(l, idx);                                       \
        typename##_remove_internal(l, idx, false);                             \
        return elem;                                                           \
    }

/*! Declares the structs and functions for an array list that stores unsigned
 * long. */
ARRAY_LIST_DECL(array_list_ul, unsigned long);

/*!
 * Constructor for array lists of type unsigned long.
 * Wrapper arround the generated constructor, that provides the call backs.
 *
 * \return A pointer to an initialized empty array list storing unsigned longs
 * with type name array_list_ul.
 */
array_list_ul*
al_ul_create(void);

/*! Declares the structs and functions for an array list that stores long. */
ARRAY_LIST_DECL(array_list_l, long);

/*!
 * Constructor for array lists of type long.
 * Wrapper arround the generated constructor, that provides the call backs.
 *
 * \return A pointer to an initialized empty array list storing longs with type
 * name array_list_l.
 */
array_list_l*
al_l_create(void);

#endif
