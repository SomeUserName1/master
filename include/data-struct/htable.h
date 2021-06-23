#ifndef HTABLE_H
#define HTABLE_H

#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/cbs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define REHASH_FILLING_RATIO  (0.6F)
#define BUCKET_START          (8)
#define TOO_MANY_BUCKETS_BITS (31U)

#define HTABLE_DEF(typename, T_key, T_val, HASH_FN, KEY_EQ)                    \
    HTABLE_CBS_TYPEDEF(typename, T_key, T_val)                                 \
    HTABLE_STRUCTS(typename, T_key, T_val)                                     \
    HTABLE_PASSTHROUGH_FN(typename, T_key, T_val)                              \
    HTABLE_BUCKET_IDX(typename, T_key)                                         \
    HTABLE_ADD_TO_BUCKETS(typename, T_key, T_val)                              \
    HTABLE_REHASH(typename)                                                    \
    HTABLE_CREATE(typename, HASH_FN, KEY_EQ)                                   \
    HTABLE_DESTROY(typename)                                                   \
    HTABLE_SIZE(typename)                                                      \
    HTABLE_INSERT(typename, T_key, T_val)                                      \
    HTABLE_REMOVE(typename, T_key)                                             \
    HTABLE_GET(typename, T_key, T_val)                                         \
    HTABLE_GET_DIRECT(typename, T_key, T_val)                                  \
    HTABLE_CONTAINS(typename, T_key, T_val)                                    \
    HTABLE_ITERATOR_CREATE(typename)                                           \
    HTABLE_ITERATOR_NEXT(typename, T_key, T_val)                               \
    HTABLE_ITERATOR_DESTROY(typename)                                          \
    HTABLE_PRINT(typename, T_key, T_val)

#define HTABLE_CBS_TYPEDEF(typename, T_key, T_val)                             \
    typedef size_t (*typename##_hash)(const T_key in, unsigned int seed);      \
    typedef bool (*typename##_keq)(const T_key first, const T_key second);     \
    typedef T_key (*typename##_kcopy)(T_key in);                               \
    typedef void (*typename##_kfree)(T_key in);                                \
    typedef void (*typename##_kprint)(const T_key in);                         \
    typedef bool (*typename##_veq)(const T_val first, const T_val second);     \
    typedef T_val (*typename##_vcopy)(T_val in);                               \
    typedef void (*typename##_vfree)(T_val in);                                \
    typedef void (*typename##_vprint)(const T_val in);

#define HTABLE_STRUCTS(typename, T_key, T_val)                                 \
    typedef struct                                                             \
    {                                                                          \
        typename##_kcopy  key_copy;                                            \
        typename##_kfree  key_free;                                            \
        typename##_kprint key_print;                                           \
        typename##_veq    value_eq;                                            \
        typename##_vcopy  value_copy;                                          \
        typename##_vfree  value_free;                                          \
        typename##_vprint value_print;                                         \
    } typename##_cbs;                                                          \
                                                                               \
    typedef struct typename##_bckt                                             \
    {                                                                          \
        T_key                   key;                                           \
        T_val                   value;                                         \
        bool                    is_used;                                       \
        struct typename##_bckt* next;                                          \
    }                                                                          \
    typename##_bucket;                                                         \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename##_hash    hash_fn;                                            \
        typename##_keq     keq;                                                \
        typename##_cbs     cbs;                                                \
        typename##_bucket* buckets;                                            \
        size_t             num_buckets;                                        \
        size_t             num_used;                                           \
        unsigned int       seed;                                               \
    } typename;                                                                \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename*          ht;                                                 \
        typename##_bucket* cur;                                                \
        size_t             idx;                                                \
    } typename##_iterator;

#define HTABLE_PASSTHROUGH_FN(typename, T_key, T_val)                          \
    static T_key typename##_passthrough_kcopy(T_key elem) { return elem; }     \
                                                                               \
    static T_val typename##_passthrough_vcopy(T_val elem) { return elem; }     \
                                                                               \
    static void typename##_passthrough_kfree(T_key elem) { elem = elem; }      \
                                                                               \
    static void typename##_passthrough_vfree(T_val elem) { elem = elem; }      \
                                                                               \
    static bool                                                                \
          typename##_passthrough_veq(const T_val first, const T_val second)    \
    {                                                                          \
        return first == second;                                                \
    }                                                                          \
                                                                               \
    static void typename##_passthrough_kprint(const T_key in)                  \
    {                                                                          \
        printf("%p\n", &in);                                                   \
    }                                                                          \
                                                                               \
    static void typename##_passthrough_vprint(const T_val in)                  \
    {                                                                          \
        printf("%p\n", &in);                                                   \
    }

#define HTABLE_BUCKET_IDX(typename, T_key)                                     \
    static size_t typename##_bucket_idx(typename* ht, T_key key)               \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - bucket_idx: Invalid Argument!\n");                \
            exit(-1);                                                          \
        }                                                                      \
        return ht->num_buckets > 0                                             \
                     ? (ht->hash_fn(key, ht->seed) % ht->num_buckets)          \
                     : ht->hash_fn(key, ht->seed);                             \
    }

#define HTABLE_ADD_TO_BUCKETS(typename, T_key, T_val)                          \
    static int typename##_add_to_bucket(                                       \
          typename* ht, T_key key, T_val val, bool rehash)                     \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - add_to_bucket: Invalid Argument!\n");             \
            exit(-1);                                                          \
        }                                                                      \
        size_t idx = typename##_bucket_idx(ht, key);                           \
                                                                               \
        if (!ht->buckets[idx].is_used) {                                       \
            if (!rehash) {                                                     \
                key = ht->cbs.key_copy(key);                                   \
                val = ht->cbs.value_copy(val);                                 \
            }                                                                  \
            ht->buckets[idx].key     = key;                                    \
            ht->buckets[idx].value   = val;                                    \
            ht->buckets[idx].is_used = true;                                   \
            if (!rehash) {                                                     \
                ht->num_used++;                                                \
            }                                                                  \
        } else {                                                               \
            typename##_bucket* cur  = ht->buckets + idx;                       \
            typename##_bucket* last = NULL;                                    \
            do {                                                               \
                if (ht->keq(key, cur->key)) {                                  \
                    ht->cbs.value_free(cur->value);                            \
                                                                               \
                    if (!rehash) {                                             \
                        val = ht->cbs.value_copy(val);                         \
                    }                                                          \
                                                                               \
                    cur->value   = val;                                        \
                    cur->is_used = true;                                       \
                    last         = NULL;                                       \
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
                    printf("htable - add_to_bucket: Memory Allocation "        \
                           "failed!\n");                                       \
                    exit(-1);                                                  \
                }                                                              \
                                                                               \
                if (!rehash) {                                                 \
                    key = ht->cbs.key_copy(key);                               \
                    val = ht->cbs.value_copy(val);                             \
                }                                                              \
                cur->key     = key;                                            \
                cur->value   = val;                                            \
                cur->is_used = true;                                           \
                last->next   = cur;                                            \
                if (!rehash) {                                                 \
                    ht->num_used++;                                            \
                }                                                              \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

#define HTABLE_REHASH(typename)                                                \
    static int typename##_rehash(typename* ht)                                 \
    {                                                                          \
        if (ht->num_used + 1                                                   \
                  < (size_t)(ht->num_buckets * REHASH_FILLING_RATIO)           \
            || ht->num_buckets >= 1UL << TOO_MANY_BUCKETS_BITS) {              \
            return 0;                                                          \
        }                                                                      \
                                                                               \
        size_t             num_buckets = ht->num_buckets;                      \
        typename##_bucket* buckets     = ht->buckets;                          \
                                                                               \
        ht->num_buckets = 2 * ht->num_buckets;                                 \
        ht->buckets     = calloc(ht->num_buckets, sizeof(*buckets));           \
                                                                               \
        if (!ht->buckets) {                                                    \
            printf("htable - rehash: Memory Allocation failed!\n");            \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_bucket* cur  = NULL;                                        \
        typename##_bucket* next = NULL;                                        \
        for (size_t i = 0; i < num_buckets; ++i) {                             \
            if (!buckets[i].is_used) {                                         \
                continue;                                                      \
            }                                                                  \
                                                                               \
            typename##_add_to_bucket(                                          \
                  ht, buckets[i].key, buckets[i].value, true);                 \
            if (buckets[i].next) {                                             \
                cur = buckets[i].next;                                         \
                do {                                                           \
                    typename##_add_to_bucket(ht, cur->key, cur->value, true);  \
                    next = cur->next;                                          \
                    free(cur);                                                 \
                    cur = next;                                                \
                } while (cur);                                                 \
            }                                                                  \
        }                                                                      \
        free(buckets);                                                         \
        return 0;                                                              \
    }

#define HTABLE_CREATE(typename, HASH_FN, KEY_EQ)                                 \
    typename* typename##_create(typename##_cbs cbs)                              \
    {                                                                            \
        typename* ht = calloc(1, sizeof(*ht));                                   \
                                                                                 \
        if (!ht) {                                                               \
            printf("htable create: failes to allocate memory!\n");               \
            exit(-1);                                                            \
        }                                                                        \
                                                                                 \
        ht->hash_fn = HASH_FN;                                                   \
        ht->keq     = KEY_EQ;                                                    \
                                                                                 \
        ht->cbs.key_copy   = cbs.key_copy == NULL ? typename##_passthrough_kcopy \
                                                  : cbs.key_copy;                \
        ht->cbs.key_free   = cbs.key_free == NULL ? typename##_passthrough_kfree \
                                                  : cbs.key_free;                \
        ht->cbs.key_print  = cbs.key_print == NULL                               \
                                   ? typename##_passthrough_kprint               \
                                   : cbs.key_print;                              \
        ht->cbs.value_eq   = cbs.value_eq == NULL ? typename##_passthrough_veq   \
                                                  : cbs.value_eq;                \
        ht->cbs.value_copy = cbs.value_copy == NULL                              \
                                   ? typename##_passthrough_vcopy                \
                                   : cbs.value_copy;                             \
        ht->cbs.value_free = cbs.value_free == NULL                              \
                                   ? typename##_passthrough_vfree                \
                                   : cbs.value_free;                             \
        ht->cbs.value_print = cbs.value_print == NULL                            \
                                    ? typename##_passthrough_vprint              \
                                    : cbs.value_print;                           \
                                                                                 \
        ht->num_buckets = BUCKET_START;                                          \
        ht->buckets     = calloc(BUCKET_START, sizeof(*ht->buckets));            \
                                                                                 \
        if (!ht->buckets) {                                                      \
            printf("htable create: Failed to allocate memory for buckets");      \
            exit(-1);                                                            \
        }                                                                        \
                                                                                 \
        ht->num_used = 0;                                                        \
        ht->seed     = rand();                                                   \
                                                                                 \
        return ht;                                                               \
    }

#define HTABLE_DESTROY(typename)                                               \
    int typename##_destroy(typename* ht)                                       \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - add_to_bucket: Invalid Argument!\n");             \
            exit(-1);                                                          \
        }                                                                      \
        typename##_bucket* cur  = NULL;                                        \
        typename##_bucket* next = NULL;                                        \
        for (size_t i = 0; i < ht->num_buckets; ++i) {                         \
            if (!ht->buckets[i].is_used) {                                     \
                continue;                                                      \
            }                                                                  \
            ht->cbs.key_free(ht->buckets[i].key);                              \
            ht->cbs.value_free(ht->buckets[i].value);                          \
                                                                               \
            next = ht->buckets[i].next;                                        \
            while (next) {                                                     \
                cur = next;                                                    \
                ht->cbs.key_free(cur->key);                                    \
                ht->cbs.value_free(cur->value);                                \
                next = cur->next;                                              \
                free(cur);                                                     \
            }                                                                  \
        }                                                                      \
        free(ht->buckets);                                                     \
        free(ht);                                                              \
                                                                               \
        return 0;                                                              \
    }

#define HTABLE_SIZE(typename)                                                  \
    size_t typename##_size(typename* ht) { return ht->num_used; }

#define HTABLE_INSERT(typename, T_key, T_val)                                  \
    int typename##_insert(typename* ht, T_key key, T_val val)                  \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - insert: Invalid Argument!\n");                    \
            exit(-1);                                                          \
        }                                                                      \
        typename##_rehash(ht);                                                 \
        return typename##_add_to_bucket(ht, key, val, false);                  \
    }

#define HTABLE_REMOVE(typename, T_key)                                         \
    int typename##_remove(typename* ht, T_key key)                             \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - remove: Invalid Argument! \n");                   \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        size_t idx = typename##_bucket_idx(ht, key);                           \
        if (!ht->buckets[idx].is_used) {                                       \
            printf("htable - remove: No such key!\n");                         \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_bucket* cur = NULL;                                         \
        if (ht->keq(ht->buckets[idx].key, key)) {                              \
            ht->cbs.key_free(ht->buckets[idx].key);                            \
            ht->cbs.value_free(ht->buckets[idx].value);                        \
            ht->buckets[idx].is_used = false;                                  \
                                                                               \
            cur = ht->buckets[idx].next;                                       \
            if (cur) {                                                         \
                ht->buckets[idx].key   = cur->key;                             \
                ht->buckets[idx].value = cur->value;                           \
                ht->buckets[idx].next  = cur->next;                            \
                free(cur);                                                     \
            }                                                                  \
            ht->num_used--;                                                    \
            return 0;                                                          \
        }                                                                      \
                                                                               \
        typename##_bucket* last = ht->buckets + idx;                           \
        cur                     = last->next;                                  \
        while (cur) {                                                          \
            if (ht->keq(cur->key, key)) {                                      \
                last->next = cur->next;                                        \
                ht->cbs.key_free(cur->key);                                    \
                ht->cbs.value_free(cur->value);                                \
                free(cur);                                                     \
                return 0;                                                      \
            }                                                                  \
            last = cur;                                                        \
            cur  = cur->next;                                                  \
        }                                                                      \
        printf("htable - remove: No such key!");                               \
        exit(-1);                                                              \
    }

#define HTABLE_GET(typename, T_key, T_val)                                     \
    int typename##_get(typename* ht, T_key key, T_val* val)                    \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - get: Invalid Argument!\n");                       \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        size_t idx = typename##_bucket_idx(ht, key);                           \
        if (!ht->buckets[idx].is_used) {                                       \
            return -1;                                                         \
        }                                                                      \
                                                                               \
        typename##_bucket* cur = ht->buckets + idx;                            \
        while (cur) {                                                          \
            if (ht->keq(cur->key, key)) {                                      \
                *val = cur->value;                                             \
                                                                               \
                return 0;                                                      \
            }                                                                  \
            cur = cur->next;                                                   \
        }                                                                      \
        return -1;                                                             \
    }

#define HTABLE_GET_DIRECT(typename, T_key, T_val)                              \
    T_val typename##_get_direct(typename* ht, T_key key)                       \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - get_direct: Invalid Argument!\n");                \
            exit(-1);                                                          \
        }                                                                      \
        T_val val;                                                             \
        if (typename##_get(ht, key, &val) < 0) {                               \
            printf("htable - get_direct: no such key!\n");                     \
            exit(-1);                                                          \
        }                                                                      \
        return val;                                                            \
    }

#define HTABLE_CONTAINS(typename, T_key, T_val)                                \
    bool typename##_contains(typename* ht, T_key key)                          \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - contains: Invalid Argument!\n");                  \
            exit(-1);                                                          \
        }                                                                      \
        T_val val;                                                             \
        return typename##_get(ht, key, &val) > -1;                             \
    }

#define HTABLE_ITERATOR_CREATE(typename)                                       \
    typename##_iterator* typename##_iterator_create(typename* ht)              \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - create_iterator: Invalid Argument!\n");           \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        typename##_iterator* hi = calloc(1, sizeof(*hi));                      \
                                                                               \
        if (!hi) {                                                             \
            printf("htable - create iterator: Memory Allocation failed!\n");   \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        hi->ht = ht;                                                           \
                                                                               \
        return hi;                                                             \
    }

#define HTABLE_ITERATOR_NEXT(typename, T_key, T_val)                           \
    int typename##_iterator_next(                                              \
          typename##_iterator* hi, T_key* key, T_val* value)                   \
    {                                                                          \
        if (!hi || !key || !value) {                                           \
            printf("htable - _iterator_next: Invalid Argument!\n");            \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        if (!hi->cur) {                                                        \
            while (hi->idx < hi->ht->num_buckets                               \
                   && !hi->ht->buckets[hi->idx].is_used) {                     \
                hi->idx++;                                                     \
            }                                                                  \
            if (hi->idx >= hi->ht->num_buckets) {                              \
                return -1;                                                     \
            }                                                                  \
            hi->cur = hi->ht->buckets + hi->idx;                               \
            hi->idx++;                                                         \
        }                                                                      \
                                                                               \
        *key    = hi->cur->key;                                                \
        *value  = hi->cur->value;                                              \
        hi->cur = hi->cur->next;                                               \
                                                                               \
        return 0;                                                              \
    }

#define HTABLE_ITERATOR_DESTROY(typename)                                      \
    void typename##_iterator_destroy(typename##_iterator* hi)                  \
    {                                                                          \
        if (!hi) {                                                             \
            return;                                                            \
        }                                                                      \
        free(hi);                                                              \
    }

#define HTABLE_PRINT(typename, T_key, T_val)                                   \
    void typename##_print(typename* ht)                                        \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - print: Invalid Argument!\n");                     \
            exit(-1);                                                          \
        }                                                                      \
        typename##_iterator* hi = typename##_iterator_create(ht);              \
                                                                               \
        T_key key;                                                             \
        T_val val;                                                             \
        while (typename##_iterator_next(hi, &key, &val) != -1) {               \
            printf("%s", "\n_______Next Entry:________ \n");                   \
            hi->ht->cbs.key_print(key);                                        \
            hi->ht->cbs.value_print(val);                                      \
            printf("%s", "_______________________\n");                         \
        }                                                                      \
        typename##_iterator_destroy(hi);                                       \
    }

HTABLE_DEF(dict_ul_ul,
           unsigned long,
           unsigned long,
           fnv_hash_ul,
           unsigned_long_eq);
dict_ul_ul_cbs d_ul_cbs = { NULL, NULL, unsigned_long_print, unsigned_long_eq,
                            NULL, NULL, unsigned_long_print };

dict_ul_ul*
d_ul_ul_create(void)
{
    return dict_ul_ul_create(d_ul_cbs);
}

HTABLE_DEF(dict_ul_int, unsigned long, int, fnv_hash_ul, unsigned_long_eq);
dict_ul_int_cbs d_int_cbs = { NULL, NULL,     unsigned_long_print, int_eq, NULL,
                              NULL, int_print };

dict_ul_int*
d_ul_int_create(void)
{
    return dict_ul_int_create(d_int_cbs);
}

HTABLE_DEF(dict_ul_node, unsigned long, node_t*, fnv_hash_ul, unsigned_long_eq);

dict_ul_node_cbs d_node_cbs = {
    NULL, NULL,      unsigned_long_print, node_equals,
    NULL, node_free, node_pretty_print
};

dict_ul_node*
d_ul_node_create(void)
{
    return dict_ul_node_create(d_node_cbs);
}

HTABLE_DEF(dict_ul_rel,
           unsigned long,
           relationship_t*,
           fnv_hash_ul,
           unsigned_long_eq);
dict_ul_rel_cbs d_rel_cbs = {
    NULL, NULL,     unsigned_long_print,      relationship_equals,
    NULL, rel_free, relationship_pretty_print
};

dict_ul_rel*
d_ul_rel_create(void)
{
    return dict_ul_rel_create(d_rel_cbs);
}

#endif
