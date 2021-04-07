#include "htable.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void*
htable_passthrough_copy(const void* elem)
{
    return (void*)elem;
}

static void
htable_passthrough_free(void* elem)
{
    elem = NULL;
}

static bool
htable_passthrough_eq(const void* first, const void* second)
{
    return first == second;
}

static void
htable_pasthrough_print(const void* in)
{
    printf("%p\n", in);
}

static size_t
htable_bucket_idx(htable_t* ht, void* key)
{
    if (!key) {
        printf("htable - bucket_idx: Invalid Argument!\n");
        exit(-1);
    }
    return ht->num_buckets > 0 ? (ht->hash_fn(key, ht->seed) % ht->num_buckets)
                               : ht->hash_fn(key, ht->seed);
}

static int
htable_add_to_bucket(htable_t* ht, void* key, void* value, bool rehash)
{
    if (!ht || !key) {
        printf("htable - add_to_bucket: Invalid Argument!\n");
        exit(-1);
    }
    size_t idx = htable_bucket_idx(ht, key);

    if (!ht->buckets[idx].key) {
        if (!rehash) {
            key = ht->cbs.key_copy(key);
            if (!key) {
                printf("htable - add_to_bucket: Key copy failed!\n");
                exit(-1);
            }
            if (value) {
                value = ht->cbs.value_copy(value);
                if (!value) {
                    printf("htable - add_to_bucket: Value copy failed!\n");
                    exit(-1);
                }
            }
        }
        ht->buckets[idx].key   = key;
        ht->buckets[idx].value = value;
        if (!rehash) {
            ht->num_used++;
        }
    } else {
        htable_bucket_t* cur  = ht->buckets + idx;
        htable_bucket_t* last = NULL;
        do {
            if (ht->keq(key, cur->key)) {
                if (cur->value) {
                    ht->cbs.value_free(cur->value);
                }
                if (!rehash && value) {
                    value = ht->cbs.value_copy(value);
                    if (!value) {
                        printf("htable - add_to_bucket: Value copy failed!\n");
                        exit(-1);
                    }
                }
                cur->value = value;
                last       = NULL;
                break;
            }
            last = cur;
            cur  = cur->next;
        } while (cur);

        if (last) {
            cur = calloc(1, sizeof(*cur->next));

            if (!cur) {
                printf("htable - add_to_bucket: Memory Allocation failed!\n");
                exit(-1);
            }

            if (!rehash) {
                key = ht->cbs.key_copy(key);
                if (key == NULL) {
                    free(cur);
                    printf("htable - add_to_bucket: Key copy failed!\n");
                    exit(-1);
                }
                if (value) {
                    value = ht->cbs.value_copy(value);
                    if (!value) {
                        free(cur);
                        free(key);
                        printf("htable - add_to_bucket: Value copy failed!\n");
                        exit(-1);
                    }
                }
            }
            cur->key   = key;
            cur->value = value;
            last->next = cur;
            if (!rehash) {
                ht->num_used++;
            }
        }
    }
    return 0;
}

static int
htable_rehash(htable_t* ht)
{
    if (ht->num_used + 1 < (size_t)(ht->num_buckets * REHASH_FILLING_RATIO)
        || ht->num_buckets >= 1UL << TOO_MANY_BUCKETS_BITS) {
        return 0;
    }

    size_t           num_buckets = ht->num_buckets;
    htable_bucket_t* buckets     = ht->buckets;

    ht->num_buckets <<= 1UL;
    ht->buckets = calloc(ht->num_buckets, sizeof(*buckets));

    if (!ht->buckets) {
        printf("htable - rehash: Memory Allocation failed!\n");
        exit(-1);
    }

    htable_bucket_t* cur  = NULL;
    htable_bucket_t* next = NULL;
    for (size_t i = 0; i < num_buckets; ++i) {
        if (!buckets[i].key) {
            continue;
        }

        htable_add_to_bucket(ht, buckets[i].key, buckets[i].value, true);
        if (buckets[i].next) {
            cur = buckets[i].next;
            do {
                htable_add_to_bucket(ht, cur->key, cur->value, true);
                next = cur->next;
                free(cur);
                cur = next;
            } while (cur);
        }
    }
    free(buckets);
    return 0;
}

htable_t*
create_htable(htable_hash fn, htable_keq keq, htable_cbs_t* cbs)
{
    if (!fn || !keq) {
        return NULL;
    }

    htable_t* ht = calloc(1, sizeof(*ht));

    if (!ht) {
        exit(-1);
    }

    ht->hash_fn = fn;
    ht->keq     = keq;

    ht->cbs.key_copy    = cbs == NULL || cbs->key_copy == NULL
                                ? htable_passthrough_copy
                                : cbs->key_copy;
    ht->cbs.key_free    = cbs == NULL || cbs->key_free == NULL
                                ? htable_passthrough_free
                                : cbs->key_free;
    ht->cbs.key_print   = cbs == NULL || cbs->key_print == NULL
                                ? htable_pasthrough_print
                                : cbs->key_print;
    ht->cbs.value_eq    = cbs == NULL || cbs->value_eq == NULL
                                ? htable_passthrough_eq
                                : cbs->value_eq;
    ht->cbs.value_copy  = cbs == NULL || cbs->value_copy == NULL
                                ? htable_passthrough_copy
                                : cbs->value_copy;
    ht->cbs.value_free  = cbs == NULL || cbs->value_free == NULL
                                ? htable_passthrough_free
                                : cbs->value_free;
    ht->cbs.value_print = cbs == NULL || cbs->value_print == NULL
                                ? htable_pasthrough_print
                                : cbs->value_print;

    ht->num_buckets = BUCKET_START;
    ht->buckets     = calloc(BUCKET_START, sizeof(*ht->buckets));

    if (!ht->buckets) {
        exit(-1);
    }

    ht->num_used = 0;

    srand(time(NULL));
    ht->seed = rand();

    return ht;
}

int
htable_destroy(htable_t* ht)
{
    if (ht == NULL) {
        printf("htable - add_to_bucket: Invalid Argument!\n");
        exit(-1);
    }
    htable_bucket_t* cur  = NULL;
    htable_bucket_t* next = NULL;
    for (size_t i = 0; i < ht->num_buckets; ++i) {
        if (!ht->buckets[i].key) {
            continue;
        }
        ht->cbs.key_free(ht->buckets[i].key);
        ht->cbs.value_free(ht->buckets[i].value);

        next = ht->buckets[i].next;
        while (next) {
            cur = next;
            ht->cbs.key_free(cur->key);
            ht->cbs.value_free(cur->value);
            next = cur->next;
            free(cur);
        }
    }
    free(ht->buckets);
    free(ht);

    return 0;
}

size_t
htable_size(htable_t* ht)
{
    return ht->num_used;
}

int
htable_insert(htable_t* ht, void* key, void* value)
{
    if (!key || !ht) {
        printf("htable - insert: Invalid Argument!\n");
        exit(-1);
    }
    htable_rehash(ht);
    return htable_add_to_bucket(ht, key, value, false);
}

int
htable_remove(htable_t* ht, void* key)
{
    if (!ht || !key) {
        printf("htable - remove: Invalid Argument!\n");
        exit(-1);
    }

    size_t idx = htable_bucket_idx(ht, key);
    if (!ht->buckets[idx].key) {
        printf("htable - remove: No such key!\n");
        exit(-1);
    }

    htable_bucket_t* cur = NULL;
    if (ht->keq(ht->buckets[idx].key, key)) {
        ht->cbs.key_free(ht->buckets[idx].key);
        ht->cbs.value_free(ht->buckets[idx].value);
        ht->buckets[idx].key = NULL;

        cur = ht->buckets[idx].next;
        if (cur) {
            ht->buckets[idx].key   = cur->key;
            ht->buckets[idx].value = cur->value;
            ht->buckets[idx].next  = cur->next;
            free(cur);
        }
        ht->num_used--;
        return 0;
    }

    htable_bucket_t* last = ht->buckets + idx;
    cur                   = last->next;
    while (cur) {
        if (ht->keq(cur->key, key)) {
            last->next = cur->next;
            ht->cbs.key_free(cur->key);
            ht->cbs.value_free(cur->value);
            free(cur);
            return 0;
        }
        last = cur;
        cur  = cur->next;
    }
    printf("htable - remove: No such key!, %p\n", key);
    exit(-1);
}

int
htable_get(htable_t* ht, void* key, void** value)
{
    if (!ht || !key) {
        printf("htable - get: Invalid Argument!\n");
        exit(-1);
    }

    size_t idx = htable_bucket_idx(ht, key);
    if (!ht->buckets[idx].key) {
        return -1;
    }

    htable_bucket_t* cur = ht->buckets + idx;
    while (cur) {
        if (ht->keq(cur->key, key)) {
            *value = cur->value;

            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

void*
htable_get_direct(htable_t* ht, void* key)
{
    if (!ht || !key) {
        printf("htable - get_direct: Invalid Argument!\n");
        exit(-1);
    }
    void* value = NULL;
    htable_get(ht, key, &value);
    return value;
}

bool
htable_contains(htable_t* ht, void* key)
{
    if (!ht || !key) {
        printf("htable - contains: Invalid Argument!\n");
        exit(-1);
    }
    void* val = NULL;
    return htable_get(ht, key, &val) > -1;
}

htable_iterator_t*
create_htable_iterator(htable_t* ht)
{
    if (!ht) {
        printf("htable - create_iterator: Invalid Argument!\n");
        exit(-1);
    }

    htable_iterator_t* hi = calloc(1, sizeof(*hi));

    if (!hi) {
        printf("htable - create iterator: Memory Allocation failed!\n");
        exit(-1);
    }

    hi->ht = ht;

    return hi;
}

int
htable_iterator_next(htable_iterator_t* hi, void** key, void** value)
{
    if (!hi || !key || !value) {
        printf("htable - create_iterator_next: Invalid Argument!\n");
        exit(-1);
    }
    if (hi->idx >= hi->ht->num_buckets) {
        return -1;
    }

    if (!hi->cur) {
        while (hi->idx < hi->ht->num_buckets && !hi->ht->buckets[hi->idx].key) {
            hi->idx++;
        }
        if (hi->idx >= hi->ht->num_buckets) {
            return -1;
        }
        hi->cur = hi->ht->buckets + hi->idx;
        hi->idx++;
    }

    *key    = hi->cur->key;
    *value  = hi->cur->value;
    hi->cur = hi->cur->next;

    return 0;
}

void
htable_iterator_destroy(htable_iterator_t* hi)
{
    if (!hi) {
        return;
    }
    free(hi);
}

void
htable_print(htable_t* ht)
{
    if (!ht) {
        printf("htable - print: Invalid Argument!\n");
        exit(-1);
    }
    htable_iterator_t* hi = create_htable_iterator(ht);

    void* key   = NULL;
    void* value = NULL;
    while (htable_iterator_next(hi, &key, &value) != -1) {
        printf("%s", "\n_______Next Entry:________ \n");
        hi->ht->cbs.key_print(key);
        hi->ht->cbs.value_print(value);
        printf("%s", "_______________________\n");
    }
    htable_iterator_destroy(hi);
}
