#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define LINKED_LIST_DECL_BASE(typename, T)                                     \
    LINKED_LIST_CBS_TYPEDEF(typename, T)                                       \
    LINKED_LIST_STRUCTS(typename, T)                                           \
    typename* typename##_create(typename##_cbs cbs);                           \
    size_t typename##_size(typename* ll);                                      \
    void typename##_destroy(typename* ll);                                     \
    void typename##_append(typename* ll, T elem);                              \
    void typename##_push(typename* ll, T elem);                                \
    void typename##_insert(typename* ll, T elem, size_t idx);                  \
    void typename##_remove(typename* ll, size_t idx);                          \
    void typename##_remove_elem(typename* ll, T elem);                         \
    bool typename##_contains(typename* ll, T elem);                            \
    int typename##_index_of(typename* ll, T elem, size_t* idx);                \
    T typename##_get(typename* ll, size_t idx);

#define LINKED_LIST_DECL(typename, T)                                          \
    LINKED_LIST_DECL_BASE(typename, T)                                         \
    T typename##_take(typename* ll, size_t idx);

#define STACK_DECL(typename, T)                                                \
    LINKED_LIST_DECL_BASE(typename, T)                                         \
    T typename##_pop(typename* ll);

#define QUEUE_DECL(typename, T)                                                \
    LINKED_LIST_DECL_BASE(typename, T)                                         \
    T typename##_pop(typename* ll);

#define LINKED_LIST_IMPL_BASE(typename, T)                                     \
    LINKED_LIST_CREATE(typename)                                               \
    LINKED_LIST_DESTROY(typename)                                              \
    LINKED_LIST_SIZE(typename)                                                 \
    LINKED_LIST_APPEND(typename, T)                                            \
    LINKED_LIST_PUSH(typename, T)                                              \
    LINKED_LIST_INSERT(typename, T)                                            \
    LINKED_LIST_REMOVE_INTERNAL(typename, T)                                   \
    LINKED_LIST_REMOVE(typename)                                               \
    LINKED_LIST_INDEX_OF(typename, T)                                          \
    LINKED_LIST_REMOVE_ELEM(typename, T)                                       \
    LINKED_LIST_CONTAINS(typename, T)                                          \
    LINKED_LIST_GET(typename, T)

#define LINKED_LIST_IMPL(typename, T)                                          \
    LINKED_LIST_IMPL_BASE(typename, T)                                         \
    LINKED_LIST_TAKE(typename, T)

#define STACK_IMPL(typename, T)                                                \
    LINKED_LIST_IMPL_BASE(typename, T)                                         \
    STACK_POP(typename, T)

#define QUEUE_IMPL(typename, T)                                                \
    LINKED_LIST_IMPL_BASE(typename, T)                                         \
    QUEUE_POP(typename, T)

#define LINKED_LIST_CBS_TYPEDEF(typename, T)                                   \
    typedef bool (*typename##_eq)(const T a, const T b);                       \
    typedef T (*typename##_copy)(T original);                                  \
    typedef void (*typename##_free)(T elem);

#define LINKED_LIST_STRUCTS(typename, T)                                       \
    typedef struct                                                             \
    {                                                                          \
        typename##_eq   lleq;                                                  \
        typename##_copy llcopy;                                                \
        typename##_free llfree;                                                \
    } typename##_cbs;                                                          \
                                                                               \
    typedef struct typename##_nd                                               \
    {                                                                          \
        T                     element;                                         \
        struct typename##_nd* prev;                                            \
        struct typename##_nd* next;                                            \
    }                                                                          \
    typename##_node;                                                           \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename##_node* head;                                                 \
        typename##_node* tail;                                                 \
        size_t           len;                                                  \
        typename##_cbs   cbs;                                                  \
    } typename;

#define LINKED_LIST_CREATE(typename)                                           \
    typename* typename##_create(typename##_cbs cbs)                            \
    {                                                                          \
        if (!cbs.lleq) {                                                       \
            printf("linked list - create: You must specify an equality "       \
                   "function for the elements!\n");                            \
            exit(-1);                                                          \
        }                                                                      \
        typename* ll;                                                          \
        ll = calloc(1, sizeof(*ll));                                           \
                                                                               \
        if (!ll) {                                                             \
            printf("linked list - create: Failed to allocate memory!\n");      \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        ll->len  = 0;                                                          \
        ll->head = NULL;                                                       \
        ll->tail = NULL;                                                       \
                                                                               \
        ll->cbs = cbs;                                                         \
                                                                               \
        return ll;                                                             \
    }

#define LINKED_LIST_SIZE(typename)                                             \
    size_t typename##_size(typename* ll)                                       \
    {                                                                          \
        if (!ll) {                                                             \
            printf("linked list - size: Invalid argument!\n");                 \
            exit(-1);                                                          \
        }                                                                      \
        return ll->len;                                                        \
    }

#define LINKED_LIST_DESTROY(typename)                                          \
    void typename##_destroy(typename* ll)                                      \
    {                                                                          \
        if (!ll) {                                                             \
            printf("linked list - destroy: Invalid argument!\n");              \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* cur = ll->head;                                       \
        typename##_node* next;                                                 \
                                                                               \
        for (size_t i = 0; i < ll->len; ++i) {                                 \
            if (ll->cbs.llfree) {                                              \
                ll->cbs.llfree(cur->element);                                  \
            }                                                                  \
            next = cur->next;                                                  \
            free(cur);                                                         \
            cur = next;                                                        \
        }                                                                      \
                                                                               \
        free(ll);                                                              \
    }

#define LINKED_LIST_APPEND(typename, T)                                        \
    void typename##_append(typename* ll, T elem)                               \
    {                                                                          \
        if (!ll) {                                                             \
            printf("Linked list - append: invalid argument!\n");               \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* node = calloc(1, sizeof(*node));                      \
                                                                               \
        if (!node) {                                                           \
            printf("Linked list - append: Failed to allocate memory!\n");      \
            exit(-1);                                                          \
        }                                                                      \
        if (ll->cbs.llcopy) {                                                  \
            elem = ll->cbs.llcopy(elem);                                       \
        }                                                                      \
                                                                               \
        node->element = elem;                                                  \
                                                                               \
        node->next = NULL;                                                     \
        node->prev = NULL;                                                     \
                                                                               \
        if (ll->tail == NULL) {                                                \
            ll->head   = node;                                                 \
            ll->tail   = node;                                                 \
            node->next = node;                                                 \
            node->prev = node;                                                 \
            ll->len++;                                                         \
        } else {                                                               \
            node->prev = ll->tail;                                             \
            node->next = ll->head;                                             \
                                                                               \
            ll->tail->next = node;                                             \
            ll->head->prev = node;                                             \
                                                                               \
            ll->tail = node;                                                   \
            ll->len++;                                                         \
        }                                                                      \
    }

#define LINKED_LIST_PUSH(typename, T)                                          \
    void typename##_push(typename* ll, T elem) { typename##_append(ll, elem); }

#define LINKED_LIST_INSERT(typename, T)                                        \
    void typename##_insert(typename* ll, T elem, size_t idx)                   \
    {                                                                          \
        if (!ll) {                                                             \
            printf("Linked list - insert: Invalid arguments!");                \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* new = calloc(1, sizeof(*new));                        \
                                                                               \
        if (!new) {                                                            \
            printf("Linked list - insert: Failed to allocate memory!\n");      \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        if (ll->cbs.llcopy) {                                                  \
            elem = ll->cbs.llcopy(elem);                                       \
        }                                                                      \
                                                                               \
        new->element = elem;                                                   \
                                                                               \
        if (ll->len == 0 && idx == 0) {                                        \
            ll->head = new;                                                    \
            ll->tail = new;                                                    \
                                                                               \
            new->prev = new;                                                   \
            new->next = new;                                                   \
            ll->len++;                                                         \
            return;                                                            \
        }                                                                      \
                                                                               \
        if (idx >= ll->len) {                                                  \
            free(new);                                                         \
            printf("linked list - insert: Index out of bounds!\n");            \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        if (idx != 0 && idx != ll->len) {                                      \
            typename##_node* cur  = ll->head;                                  \
            typename##_node* next = ll->head;                                  \
                                                                               \
            for (size_t i = 0; i < idx; ++i) {                                 \
                cur  = next;                                                   \
                next = cur->next;                                              \
            }                                                                  \
                                                                               \
            new->next = next;                                                  \
            new->prev = cur;                                                   \
                                                                               \
            cur->next  = new;                                                  \
            next->prev = new;                                                  \
        } else {                                                               \
            new->prev = ll->tail;                                              \
            new->next = ll->head;                                              \
                                                                               \
            ll->head->prev = new;                                              \
            ll->tail->next = new;                                              \
                                                                               \
            if (idx == 0) {                                                    \
                ll->head = new;                                                \
            } else {                                                           \
                ll->tail = new;                                                \
            }                                                                  \
        }                                                                      \
                                                                               \
        ll->len++;                                                             \
    }

#define LINKED_LIST_REMOVE_INTERNAL(typename, T)                               \
    static void                                                                \
          typename##_remove_internal(typename* ll, size_t idx, bool free_flag) \
    {                                                                          \
        if (!ll || idx >= ll->len) {                                           \
            printf("Linked list - remove internal: Invalid arguemnts!\n");     \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        if (ll->len == 1 && idx == 0) {                                        \
            if (free_flag && ll->cbs.llfree) {                                 \
                ll->cbs.llfree(ll->head->element);                             \
            }                                                                  \
            free(ll->head);                                                    \
            ll->head = NULL;                                                   \
            ll->tail = NULL;                                                   \
            ll->len--;                                                         \
        } else if (idx == 0) {                                                 \
            if (free_flag && ll->cbs.llfree) {                                 \
                ll->cbs.llfree(ll->head->element);                             \
            }                                                                  \
            typename##_node* temp = ll->head->next;                            \
            free(ll->head);                                                    \
            ll->head       = temp;                                             \
            ll->head->prev = ll->tail;                                         \
            ll->len--;                                                         \
        } else if (idx == ll->len - 1) {                                       \
            if (free_flag && ll->cbs.llfree) {                                 \
                ll->cbs.llfree(ll->tail->element);                             \
            }                                                                  \
            typename##_node* temp = ll->tail->prev;                            \
            free(ll->tail);                                                    \
            ll->tail       = temp;                                             \
            ll->tail->next = ll->head;                                         \
            ll->len--;                                                         \
        } else {                                                               \
            typename##_node* cur;                                              \
            typename##_node* next = ll->head;                                  \
            for (size_t i = 0; i < idx; ++i) {                                 \
                cur  = next;                                                   \
                next = cur->next;                                              \
            }                                                                  \
                                                                               \
            typename##_node* remove = next;                                    \
            next                    = remove->next;                            \
            cur->next               = next;                                    \
            next->prev              = cur;                                     \
            ll->len--;                                                         \
                                                                               \
            if (free_flag && ll->cbs.llfree) {                                 \
                ll->cbs.llfree(remove->element);                               \
            }                                                                  \
            free(remove);                                                      \
        }                                                                      \
    }

#define LINKED_LIST_REMOVE(typename)                                           \
    void typename##_remove(typename* ll, size_t idx)                           \
    {                                                                          \
        return typename##_remove_internal(ll, idx, true);                      \
    }

#define LINKED_LIST_REMOVE_ELEM(typename, T)                                   \
    void typename##_remove_elem(typename* ll, T elem)                          \
    {                                                                          \
        size_t idx;                                                            \
        if (typename##_index_of(ll, elem, &idx) <= -1) {                       \
            printf("Linked List - remove elem: Element not in the list!\n");   \
            exit(-1);                                                          \
        }                                                                      \
        return typename##_remove_internal(ll, idx, true);                      \
    }

#define LINKED_LIST_INDEX_OF(typename, T)                                      \
    int typename##_index_of(typename* ll, T elem, size_t* idx)                 \
    {                                                                          \
        if (!ll || !idx || ll->len == 0) {                                     \
            printf("linked list - index of: invalid arguments!\n");            \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* cur = ll->head;                                       \
        for (size_t i = 0; i < ll->len; ++i) {                                 \
            if (ll->cbs.lleq(cur->element, elem)) {                            \
                *idx = i;                                                      \
                return 0;                                                      \
            }                                                                  \
            cur = cur->next;                                                   \
        }                                                                      \
        return -1;                                                             \
    }

#define LINKED_LIST_CONTAINS(typename, T)                                      \
    bool typename##_contains(typename* ll, T elem)                             \
    {                                                                          \
        size_t idx;                                                            \
        return (typename##_index_of(ll, elem, &idx) > -1);                     \
    }

#define LINKED_LIST_GET(typename, T)                                           \
    T typename##_get(typename* ll, size_t idx)                                 \
    {                                                                          \
        if (!ll || idx >= ll->len) {                                           \
            printf("Linked List - get: Invalid Arguments!\n");                 \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* cur = ll->head;                                       \
        for (size_t i = 0; i < idx; ++i) {                                     \
            cur = cur->next;                                                   \
        }                                                                      \
        return cur->element;                                                   \
    }

#define LINKED_LIST_TAKE(typename, T)                                          \
    T typename##_take(typename* ll, size_t idx)                                \
    {                                                                          \
        T result = typename##_get(ll, idx);                                    \
        typename##_remove_internal(ll, idx, false);                            \
                                                                               \
        return result;                                                         \
    }

#define QUEUE_POP(typename, T)                                                 \
    T typename##_pop(typename* ll)                                             \
    {                                                                          \
        T result = ll->head->element;                                          \
        typename##_remove_internal(ll, 0, false);                              \
                                                                               \
        return result;                                                         \
    }

#define STACK_POP(typename, T)                                                 \
    T typename##_pop(typename* ll)                                             \
    {                                                                          \
        T result = ll->tail->element;                                          \
        typename##_remove_internal(ll, ll->len - 1, false);                    \
                                                                               \
        return result;                                                         \
    }

LINKED_LIST_DECL(llist_ul, unsigned long);

llist_ul*
ll_ul_create(void);

QUEUE_DECL(queue_ul, unsigned long);

queue_ul*
q_ul_create(void);

STACK_DECL(stack_ul, unsigned long);

stack_ul*
st_ul_create(void);

#endif
