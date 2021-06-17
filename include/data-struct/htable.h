#ifndef HTABLE_H
#define HTABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define REHASH_FILLING_RATIO  (0.6F)
#define BUCKET_START          (8)
#define TOO_MANY_BUCKETS_BITS (31U)

#define HTABLE_DEF(typename, T_key, T_val)                                     \
                                                                               \
    typedef size_t (*typename##_hash)(const T_key in, unsigned int seed);      \
    typedef bool (*typename##_keq)(const T_key first, const T_key second);     \
    typedef T_key (*typename##_kcopy)(const T_key in);                         \
    typedef void (*typename##_kfree)(T_key in);                                \
    typedef void (*typename##_kprint)(const T_key in);                         \
    typedef bool (*typename##_veq)(const T_val* first, const T_val* second);   \
    typedef T_val (*typename##_vcopy)(const T_val in);                         \
    typedef void (*typename##_vfree)(T_val in);                                \
    typedef void (*typename##_vprint)(const T_val in);                         \
                                                                               \
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
        bool               is_set;                                             \
        unsigned int       seed;                                               \
    } typename;                                                                \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename ht;                                                           \
        typename##_bucket* cur;                                                \
        size_t             idx;                                                \
    } typename##_iterator;                                                     \
                                                                               \
    static T_key typename##_passthrough_kcopy(const T_key elem)                \
    {                                                                          \
        return elem;                                                           \
    }                                                                          \
                                                                               \
    static T_val typename##_passthrough_vcopy(const T_val elem)                \
    {                                                                          \
        return elem;                                                           \
    }                                                                          \
                                                                               \
    static void typename##_passthrough_kfree(T_key elem) { &elem = NULL; }     \
                                                                               \
    static void typename##_passthrough_vfree(T_val elem) { &elem = NULL; }     \
                                                                               \
    static bool                                                                \
          typename##_passthrough_veq(const T_val first, const T_val second)    \
    {                                                                          \
        return first == second;                                                \
    }                                                                          \
                                                                               \
    static void typename##_pasthrough_kprint(const T_key in)                   \
    {                                                                          \
        printf("%p\n", &in);                                                   \
    }                                                                          \
                                                                               \
    static void typename##_passthrough_vprint(const T_val in)                  \
    {                                                                          \
        printf("%p\n", &in);                                                   \
    }                                                                          \
                                                                               \
    static size_t typename##_bucket_idx(typename* ht, T_key key)               \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - bucket_idx: Invalid Argument!\n");                \
            exit(-1);                                                          \
        }                                                                      \
        return ht->num_buckets > 0                                             \
                     ? (ht->hash_fn(key, ht->seed) % ht->num_buckets)          \
                     : ht->hash_fn(key, ht->seed);                             \
    }                                                                          \
                                                                               \
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
                if (!ht.is_set) {                                              \
                    value = ht->cbs.value_copy(value);                         \
                }                                                              \
            }                                                                  \
            ht->buckets[idx].key   = key;                                      \
            ht->buckets[idx].value = value;                                    \
            if (!rehash) {                                                     \
                ht->num_used++;                                                \
            }                                                                  \
        } else {                                                               \
            typename##_bucket* cur  = ht->buckets + idx;                       \
            typename##_bucket* last = NULL;                                    \
            do {                                                               \
                if (ht->keq(key, cur->key)) {                                  \
                    if (!ht.is_set) {                                          \
                        ht->cbs.value_free(cur->value);                        \
                                                                               \
                        if (!rehash) {                                         \
                            value = ht->cbs.value_copy(value);                 \
                        }                                                      \
                    }                                                          \
                                                                               \
                    cur->value = value;                                        \
                    last       = NULL;                                         \
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
                    if (key == NULL) {                                         \
                        free(cur);                                             \
                        printf("htable - add_to_bucket: Key copy failed!\n");  \
                        exit(-1);                                              \
                    }                                                          \
                    if (!ht.is_set) {                                          \
                        value = ht->cbs.value_copy(value);                     \
                    }                                                          \
                }                                                              \
                cur->key   = key;                                              \
                cur->value = value;                                            \
                last->next = cur;                                              \
                if (!rehash) {                                                 \
                    ht->num_used++;                                            \
                }                                                              \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }                                                                          \
                                                                               \
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
    }                                                                          \
                                                                               \
    typename* typename##_create(                                               \
          typename##_hash fn, typename##_keq keq, typename##_cbs* cbs)         \
    {                                                                          \
        if (!fn || !keq) {                                                     \
            printf("create htable: No hash function or key equality function " \
                   "provided!\n");                                             \
            return exit(-1);                                                   \
        }                                                                      \
                                                                               \
        typename* ht = calloc(1, sizeof(*ht));                                 \
                                                                               \
        if (!ht) {                                                             \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        ht->hash_fn = fn;                                                      \
        ht->keq     = keq;                                                     \
                                                                               \
        ht->cbs.key_copy    = cbs == NULL || cbs->key_copy == NULL             \
                                    ? typename##_passthrough_kcopy             \
                                    : cbs->key_copy;                           \
        ht->cbs.key_free    = cbs == NULL || cbs->key_free == NULL             \
                                    ? typename##_passthrough_kfree             \
                                    : cbs->key_free;                           \
        ht->cbs.key_print   = cbs == NULL || cbs->key_print == NULL            \
                                    ? typename##_pasthrough_kprint             \
                                    : cbs->key_print;                          \
        ht->cbs.value_eq    = cbs == NULL || cbs->value_eq == NULL             \
                                    ? typename##_passthrough_veq               \
                                    : cbs->value_eq;                           \
        ht->cbs.value_copy  = cbs == NULL || cbs->value_copy == NULL           \
                                    ? typename##_passthrough_vcopy             \
                                    : cbs->value_copy;                         \
        ht->cbs.value_free  = cbs == NULL || cbs->value_free == NULL           \
                                    ? typename##_passthrough_vfree             \
                                    : cbs->value_free;                         \
        ht->cbs.value_print = cbs == NULL || cbs->value_print == NULL          \
                                    ? typename##_pasthrough_print              \
                                    : cbs->value_print;                        \
                                                                               \
        ht->num_buckets = BUCKET_START;                                        \
        ht->buckets     = calloc(BUCKET_START, sizeof(*ht->buckets));          \
                                                                               \
        if (!ht->buckets) {                                                    \
            printf("htable create: Failed to allocate memory for buckets");    \
            exit(-1);                                                          \
        }                                                                      \
                                                                               \
        ht->num_used = 0;                                                      \
                                                                               \
        ht->seed = rand();                                                     \
                                                                               \
        return ht;                                                             \
    }                                                                          \
                                                                               \
    int typename##_destroy(typename* ht)                                       \
    {                                                                          \
        if (ht == NULL) {                                                      \
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
    }                                                                          \
                                                                               \
    size_t typename##_size(typename* ht) { return ht->num_used; }              \
                                                                               \
    int typename##_insert(typename* ht, T_key key, T_val val)                  \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - insert: Invalid Argument!\n");                    \
            exit(-1);                                                          \
        }                                                                      \
        typename##_rehash(ht);                                                 \
        return typename##_add_to_bucket(ht, key, value, false);                \
    }                                                                          \
                                                                               \
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
            ht->buckets[idx].key = NULL;                                       \
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
        printf("htable - remove: No such key!, %p\n", key);                    \
        exit(-1);                                                              \
    }                                                                          \
                                                                               \
    int typename##_get(typename* ht, T_key key, T_val* value)                  \
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
                *value = cur->value;                                           \
                                                                               \
                return 0;                                                      \
            }                                                                  \
            cur = cur->next;                                                   \
        }                                                                      \
        return -1;                                                             \
    }                                                                          \
                                                                               \
    T_val typename##_get_direct(typename* ht, T_key key)                       \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - get_direct: Invalid Argument!\n");                \
            exit(-1);                                                          \
        }                                                                      \
        T_val val = NULL;                                                      \
        typename##_get(ht, key, &value);                                       \
        return value;                                                          \
    }                                                                          \
                                                                               \
    bool typename##_contains(typename* ht, T_key key)                          \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - contains: Invalid Argument!\n");                  \
            exit(-1);                                                          \
        }                                                                      \
        T_val val = NULL;                                                      \
        return typename##_get(ht, key, &val) > -1;                             \
    }                                                                          \
                                                                               \
    typename##_iterator* typename##_create_iterator(typename* ht)              \
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
    }                                                                          \
                                                                               \
    int typename##_iterator_next(                                              \
          typename##_iterator* hi, T_key* key, T_val* value)                   \
    {                                                                          \
        if (!hi || !key || !value) {                                           \
            printf("htable - create_iterator_next: Invalid Argument!\n");      \
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
    }                                                                          \
                                                                               \
    void typename##_iterator_destroy(typename##_iterator* hi)                  \
    {                                                                          \
        if (!hi) {                                                             \
            return;                                                            \
        }                                                                      \
        free(hi);                                                              \
    }                                                                          \
                                                                               \
    void typename##_print(typename* ht)                                        \
    {                                                                          \
        if (!ht) {                                                             \
            printf("htable - print: Invalid Argument!\n");                     \
            exit(-1);                                                          \
        }                                                                      \
        typename##_iterator* hi = create_typename##_iterator(ht);              \
                                                                               \
        T_key key = NULL;                                                      \
        T_val val = NULL;                                                      \
        while (typename##_iterator_next(hi, &key, &value) != -1) {             \
            printf("%s", "\n_______Next Entry:________ \n");                   \
            hi->ht->cbs.key_print(key);                                        \
            hi->ht->cbs.value_print(value);                                    \
            printf("%s", "_______________________\n");                         \
        }                                                                      \
        typename##_iterator_destroy(hi);                                       \
    }

#endif
