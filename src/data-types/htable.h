#ifndef __HTABLE_H__
#define __HTABLE_H__

#include <stdbool.h>
#include <stddef.h>

const size_t BUCKET_START = 8;

typedef unsigned int (*htable_hash)(const void* in, unsigned int seed);
typedef bool (*htable_keq)(const void* first, const void* second);
typedef void* (*htable_kcopy)(const void* in);
typedef void (*htable_kfree)(void* in);
typedef bool (*htable_veq)(const void* first, const void* second);
typedef void* (*htable_vcopy)(const void* in);
typedef void (*htable_vfree)(void* in);

typedef struct htable_cbs {
    htable_kcopy key_copy;
    htable_kfree key_free;
    htable_veq value_eq;
    htable_vcopy value_copy;
    htable_vfree value_free;
} htable_cbs_t;

typedef struct htable_bucket {
    void* key;
    void* value;
    struct htable_bucket* next;
} htable_bucket_t;

typedef struct htable {
    htable_hash hash_fn;
    htable_keq keq;
    htable_cbs_t cbs;
    htable_bucket_t* buckets;
    size_t num_buckets;
    size_t num_used;
    unsigned int seed;
} htable_t;

typedef struct htable_iterator {
    htable_t* ht;
    htable_bucket_t* cur;
    size_t idx;
} htable_iterator_t;

htable_t* create_htable(htable_hash fn, htable_keq, htable_cbs_t* cbs);
int htable_destroy(htable_t* ht);

int htable_insert(htable_t* ht, void* key, void* value);
int htable_remove(htable_t* ht, void* key);

int htable_get(htable_t* ht, void* key, void** value);
void* htable_get_direct(htable_t* ht, void* key);
bool htable_contains(htable_t* ht, void* key);

htable_iterator_t* create_htable_iterator(htable_t* ht);
int htable_iterator_next(htable_iterator_t* hi, void** key, void** value);
void htable_iterator_destroy(htable_iterator_t* hi);
#endif
