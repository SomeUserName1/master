#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define REHASH_FILLING_RATIO_S  (0.8F)
#define BUCKET_START_S          (32)
#define TOO_MANY_BUCKETS_BITS_S (31U)

#define SET_DECL(typename, T)                                                  \
    SET_CBS_TYPEDEF(typename, T)                                               \
    SET_STRUCTS(typename, T)                                                   \
    typename* typename##_create(typename##_cbs cbs);                           \
    int typename##_destroy(typename* s);                                       \
    size_t typename##_size(typename* s);                                       \
    int typename##_insert(typename* s, T elem);                                \
    int typename##_remove(typename* s, T elem);                                \
    bool typename##_contains(typename* s, T elem);                             \
    typename##_iterator* typename##_iterator_create(typename* s);              \
    int typename##_iterator_next(typename##_iterator* hi, T* elem);            \
    void typename##_iterator_destroy(typename##_iterator* hi);                 \
    void typename##_print(typename* s);

#define SET_IMPL(typename, T, HASH_FN, EQ)                                     \
    SET_BUCKET_IDX(typename, T)                                                \
    SET_ADD_TO_BUCKETS(typename, T)                                            \
    SET_REHASH(typename)                                                       \
    SET_CREATE(typename, HASH_FN, EQ)                                          \
    SET_DESTROY(typename)                                                      \
    SET_SIZE(typename)                                                         \
    SET_INSERT(typename, T)                                                    \
    SET_REMOVE(typename, T)                                                    \
    SET_CONTAINS(typename, T)                                                  \
    SET_ITERATOR_CREATE(typename)                                              \
    SET_ITERATOR_NEXT(typename, T)                                             \
    SET_ITERATOR_DESTROY(typename)                                             \
    SET_PRINT(typename, T)

#define SET_CBS_TYPEDEF(typename, T)                                           \
    typedef size_t (*typename##_hash)(const T in, unsigned int seed);          \
    typedef bool (*typename##_eq)(const T first, const T second);              \
    typedef T (*typename##_copy)(T in);                                        \
    typedef void (*typename##_free)(T in);                                     \
    typedef void (*typename##_pretty_print)(const T in);

#define SET_STRUCTS(typename, T)                                               \
    typedef struct                                                             \
    {                                                                          \
        typename##_copy         copy;                                          \
        typename##_free         free;                                          \
        typename##_pretty_print print;                                         \
    } typename##_cbs;                                                          \
                                                                               \
    typedef struct typename##_bckt                                             \
    {                                                                          \
        T                       elem;                                          \
        bool                    is_used;                                       \
        struct typename##_bckt* next;                                          \
    }                                                                          \
    typename##_bucket;                                                         \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename##_hash    hash_fn;                                            \
        typename##_eq      eq;                                                 \
        typename##_cbs     cbs;                                                \
        typename##_bucket* buckets;                                            \
        size_t             num_buckets;                                        \
        size_t             num_used;                                           \
        unsigned int       seed;                                               \
    } typename;                                                                \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename*          s;                                                  \
        typename##_bucket* cur;                                                \
        size_t             idx;                                                \
    } typename##_iterator;

#define SET_CREATE(typename, HASH_FN, EQ)                                      \
    typename* typename##_create(typename##_cbs cbs)                            \
    {                                                                          \
        typename* s = calloc(1, sizeof(*s));                                   \
                                                                               \
        if (!s) {                                                              \
            printf("set - create: failes to allocate memory!\n");              \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        s->hash_fn     = HASH_FN;                                              \
        s->eq          = EQ;                                                   \
        s->cbs         = cbs;                                                  \
        s->num_buckets = BUCKET_START_S;                                       \
        s->buckets     = calloc(BUCKET_START_S, sizeof(*s->buckets));          \
                                                                               \
        if (!s->buckets) {                                                     \
            printf("set - create: Failed to allocate memory for buckets");     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        s->num_used = 0;                                                       \
        s->seed     = rand();                                                  \
                                                                               \
        return s;                                                              \
    }

#define SET_DESTROY(typename)                                                  \
    int typename##_destroy(typename* s)                                        \
    {                                                                          \
        if (!s) {                                                              \
            printf("s - add_to_bucket: Invalid Argument!\n");                  \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename##_bucket* cur  = NULL;                                        \
        typename##_bucket* next = NULL;                                        \
        for (size_t i = 0; i < s->num_buckets; ++i) {                          \
            if (!s->buckets[i].is_used) {                                      \
                continue;                                                      \
            }                                                                  \
            if (s->cbs.free) {                                                 \
                s->cbs.free(s->buckets[i].elem);                               \
            }                                                                  \
                                                                               \
            next = s->buckets[i].next;                                         \
            while (next) {                                                     \
                cur = next;                                                    \
                if (s->cbs.free) {                                             \
                    s->cbs.free(cur->elem);                                    \
                }                                                              \
                next = cur->next;                                              \
                free(cur);                                                     \
            }                                                                  \
        }                                                                      \
        free(s->buckets);                                                      \
        free(s);                                                               \
                                                                               \
        return 0;                                                              \
    }

#define SET_SIZE(typename)                                                     \
    size_t typename##_size(typename* s) { return s->num_used; }

#define SET_BUCKET_IDX(typename, T)                                            \
    static size_t typename##_bucket_idx(typename* s, T elem)                   \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - bucket_idx: Invalid Argument!\n");                   \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        return s->num_buckets > 0                                              \
                     ? (s->hash_fn(elem, s->seed) % s->num_buckets)            \
                     : s->hash_fn(elem, s->seed);                              \
    }

#define SET_ADD_TO_BUCKETS(typename, T)                                        \
    static int typename##_add_to_bucket(typename* s, T elem, bool rehash)      \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - add_to_bucket: Invalid Argument!\n");                \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        size_t idx = typename##_bucket_idx(s, elem);                           \
                                                                               \
        if (!s->buckets[idx].is_used) {                                        \
            if (!rehash && s->cbs.free) {                                      \
                elem = s->cbs.copy(elem);                                      \
            }                                                                  \
            s->buckets[idx].elem    = elem;                                    \
            s->buckets[idx].is_used = true;                                    \
            if (!rehash) {                                                     \
                s->num_used++;                                                 \
            }                                                                  \
        } else {                                                               \
            typename##_bucket* cur  = s->buckets + idx;                        \
            typename##_bucket* last = NULL;                                    \
            do {                                                               \
                if (s->eq(elem, cur->elem)) {                                  \
                    last = NULL;                                               \
                    break;                                                     \
                }                                                              \
                last = cur;                                                    \
                cur  = cur->next;                                              \
            } while (cur);                                                     \
                                                                               \
            if (last) {                                                        \
                cur = calloc(1, sizeof(*cur->next));                           \
                                                                               \
                if (!cur) {                                                    \
                    printf("set - add_to_bucket: Memory Allocation "           \
                           "failed!\n");                                       \
                    exit(EXIT_FAILURE);                                        \
                }                                                              \
                                                                               \
                if (!rehash && s->cbs.copy) {                                  \
                    elem = s->cbs.copy(elem);                                  \
                }                                                              \
                cur->elem    = elem;                                           \
                cur->is_used = true;                                           \
                last->next   = cur;                                            \
                if (!rehash) {                                                 \
                    s->num_used++;                                             \
                }                                                              \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

#define SET_REHASH(typename)                                                   \
    static int typename##_rehash(typename* s)                                  \
    {                                                                          \
        if (s->num_used + 1                                                    \
                  < (size_t)(s->num_buckets * REHASH_FILLING_RATIO_S)          \
            || s->num_buckets >= 1UL << TOO_MANY_BUCKETS_BITS_S) {             \
            return 0;                                                          \
        }                                                                      \
                                                                               \
        size_t             num_buckets = s->num_buckets;                       \
        typename##_bucket* buckets     = s->buckets;                           \
                                                                               \
        s->num_buckets = 2 * s->num_buckets;                                   \
        s->buckets     = calloc(s->num_buckets, sizeof(*buckets));             \
                                                                               \
        if (!s->buckets) {                                                     \
            printf("set - rehash: Memory Allocation failed!\n");               \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        typename##_bucket* cur  = NULL;                                        \
        typename##_bucket* next = NULL;                                        \
        for (size_t i = 0; i < num_buckets; ++i) {                             \
            if (!buckets[i].is_used) {                                         \
                continue;                                                      \
            }                                                                  \
                                                                               \
            typename##_add_to_bucket(s, buckets[i].elem, true);                \
            if (buckets[i].next) {                                             \
                cur = buckets[i].next;                                         \
                do {                                                           \
                    typename##_add_to_bucket(s, cur->elem, true);              \
                    next = cur->next;                                          \
                    free(cur);                                                 \
                    cur = next;                                                \
                } while (cur);                                                 \
            }                                                                  \
        }                                                                      \
        free(buckets);                                                         \
        return 0;                                                              \
    }

#define SET_INSERT(typename, T)                                                \
    int typename##_insert(typename* s, T elem)                                 \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - insert: Invalid Argument!\n");                       \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename##_rehash(s);                                                  \
        return typename##_add_to_bucket(s, elem, false);                       \
    }

#define SET_REMOVE(typename, T)                                                \
    int typename##_remove(typename* s, T elem)                                 \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - remove: Invalid Argument! \n");                      \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        size_t idx = typename##_bucket_idx(s, elem);                           \
        if (!s->buckets[idx].is_used) {                                        \
            printf("set - remove: No such element!\n");                        \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        typename##_bucket* cur = NULL;                                         \
        if (s->eq(s->buckets[idx].elem, elem)) {                               \
            if (s->cbs.free) {                                                 \
                s->cbs.free(s->buckets[idx].elem);                             \
            }                                                                  \
            s->buckets[idx].is_used = false;                                   \
                                                                               \
            cur = s->buckets[idx].next;                                        \
            if (cur) {                                                         \
                s->buckets[idx].elem = cur->elem;                              \
                s->buckets[idx].next = cur->next;                              \
                free(cur);                                                     \
            }                                                                  \
            s->num_used--;                                                     \
            return 0;                                                          \
        }                                                                      \
                                                                               \
        typename##_bucket* last = s->buckets + idx;                            \
        cur                     = last->next;                                  \
        while (cur) {                                                          \
            if (s->eq(cur->elem, elem)) {                                      \
                last->next = cur->next;                                        \
                if (s->cbs.free) {                                             \
                    s->cbs.free(cur->elem);                                    \
                }                                                              \
                free(cur);                                                     \
                return 0;                                                      \
            }                                                                  \
            last = cur;                                                        \
            cur  = cur->next;                                                  \
        }                                                                      \
        printf("set - remove: No such element!");                              \
        exit(EXIT_FAILURE);                                                    \
    }

#define SET_CONTAINS(typename, T)                                              \
    bool typename##_contains(typename* s, T elem)                              \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - get: Invalid Argument!\n");                          \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        size_t idx = typename##_bucket_idx(s, elem);                           \
        if (!s->buckets[idx].is_used) {                                        \
            return false;                                                      \
        }                                                                      \
                                                                               \
        typename##_bucket* cur = s->buckets + idx;                             \
        while (cur) {                                                          \
            if (s->eq(cur->elem, elem)) {                                      \
                return true;                                                   \
            }                                                                  \
            cur = cur->next;                                                   \
        }                                                                      \
        return false;                                                          \
    }

#define SET_ITERATOR_CREATE(typename)                                          \
    typename##_iterator* typename##_iterator_create(typename* s)               \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - create_iterator: Invalid Argument!\n");              \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        typename##_iterator* hi = calloc(1, sizeof(*hi));                      \
                                                                               \
        if (!hi) {                                                             \
            printf("set - create iterator: Memory Allocation failed!\n");      \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        hi->s = s;                                                             \
                                                                               \
        return hi;                                                             \
    }

#define SET_ITERATOR_NEXT(typename, T)                                         \
    int typename##_iterator_next(typename##_iterator* hi, T* elem)             \
    {                                                                          \
        if (!hi || !elem) {                                                    \
            printf("set - iterator_next: Invalid Argument!\n");                \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        if (!hi->cur) {                                                        \
            while (hi->idx < hi->s->num_buckets                                \
                   && !hi->s->buckets[hi->idx].is_used) {                      \
                hi->idx++;                                                     \
            }                                                                  \
            if (hi->idx >= hi->s->num_buckets) {                               \
                return -1;                                                     \
            }                                                                  \
            hi->cur = hi->s->buckets + hi->idx;                                \
            hi->idx++;                                                         \
        }                                                                      \
                                                                               \
        *elem   = hi->cur->elem;                                               \
        hi->cur = hi->cur->next;                                               \
                                                                               \
        return 0;                                                              \
    }

#define SET_ITERATOR_DESTROY(typename)                                         \
    void typename##_iterator_destroy(typename##_iterator* hi)                  \
    {                                                                          \
        if (!hi) {                                                             \
            return;                                                            \
        }                                                                      \
        free(hi);                                                              \
    }

#define SET_PRINT(typename, T)                                                 \
    void typename##_print(typename* s)                                         \
    {                                                                          \
        if (!s) {                                                              \
            printf("set - print: Invalid Argument!\n");                        \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        typename##_iterator* hi = typename##_iterator_create(s);               \
                                                                               \
        T elem;                                                                \
        while (typename##_iterator_next(hi, &elem) != -1) {                    \
            printf("%s", "\n_______Next Entry:________ \n");                   \
            if (s->cbs.print) {                                                \
                hi->s->cbs.print(elem);                                        \
            } else {                                                           \
                printf("No printf function provided!\n");                      \
            }                                                                  \
            printf("%s", "_______________________\n");                         \
        }                                                                      \
        typename##_iterator_destroy(hi);                                       \
    }

SET_DECL(set_ul, unsigned long);

set_ul*
s_ul_create(void);

#endif
