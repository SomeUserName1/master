#include "dict_ul.h"
#include "ul_helpers.h"

#include <stdlib.h>

static unsigned int fnv_hash_node(const void* in, unsigned int seed) {
    unsigned int h = seed;
    unsigned int prime = 0xFDCFB7;
    unsigned long ul = *((unsigned long*) in);
    unsigned int low = ul & 0xFFFFFFFF;
    unsigned int high = ul >> 32;

    h = (h ^ high) * prime;
    h = (h ^ low) * prime;

    return h;
}

static bool int_eq(const void* first, const void* second) {
    return (*((int*) first) == *((int*) second));
}

static void* int_copy(const void* original) {
    int* copy = malloc(sizeof(*copy));
    *copy = *((int*)original);
    return copy;
}

dict_ul_ul_t* create_dict_ul_ul() {
    htable_hash hash = fnv_hash_node;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        unsigned_long_eq,
        unsigned_long_copy,
        free
    };

    return (dict_ul_ul_t*) create_htable(hash, keq, &cbs);
}

int dict_ul_ul_destroy(dict_ul_ul_t* ht) {
    return htable_destroy((htable_t*) ht);
}

int dict_ul_ul_insert(dict_ul_ul_t* ht, unsigned long key, unsigned long value) {
    return htable_insert((htable_t*) ht, (void*) &key, (void*) &value);
}

int dict_ul_ul_remove(dict_ul_ul_t* ht, unsigned long key) {
    return  htable_remove((htable_t*) ht, (void*) &key);
}

int dict_ul_ul_get(dict_ul_ul_t* ht, unsigned long key, unsigned long* value) {
    return htable_get((htable_t*) ht, (void*) &key, (void**) &value);
}

unsigned long dict_ul_ul_get_direct(dict_ul_ul_t* ht, unsigned long key) {
    return *((unsigned long*) htable_get_direct((htable_t*) ht, (void*) &key));
}

dict_ul_ul_iterator_t* create_dict_ul_ul_iterator(dict_ul_ul_t* ht) {
    return (dict_ul_ul_iterator_t*) create_htable_iterator((htable_t*) ht);
}

int dict_ul_ul_iterator_next(dict_ul_ul_iterator_t* hi, unsigned long* key, unsigned long* value) {
    return htable_iterator_next((htable_iterator_t*) hi, (void**) &key, (void**) &value);
}

void dict_ul_ul_iterator_destroy(dict_ul_ul_iterator_t* hi) {
    return htable_iterator_destroy((htable_iterator_t*) hi);
}


dict_ul_int_t* create_dict_ul_int() {
    htable_hash hash = fnv_hash_node;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        int_eq,
        int_copy,
        free
    };

    return (dict_ul_int_t*) create_htable(hash, keq, &cbs);
}

int dict_ul_int_destroy(dict_ul_int_t* ht) {
    return htable_destroy((htable_t*) ht);
}

int dict_ul_int_insert(dict_ul_int_t* ht, unsigned long key, int value) {
    return htable_insert((htable_t*) ht, (void*) &key, (void*) &value);
}

int dict_ul_int_remove(dict_ul_int_t* ht, unsigned long key) {
    return htable_remove((htable_t*) ht, (void*) &key);
}

int dict_ul_int_get(dict_ul_int_t* ht, unsigned long key, int* value) {
    return htable_get((htable_t*) ht, (void*) &key, (void**) &value);
}

int dict_ul_int_get_direct(dict_ul_int_t* ht, unsigned long key) {
    return *((int*) htable_get_direct((htable_t*) ht, (void*) &key));
}

dict_ul_int_iterator_t* create_dict_ul_int_iterator(htable_t* ht) {
    return (dict_ul_int_iterator_t*) create_htable_iterator((htable_t*) ht);
}

int dict_ul_int_iterator_next(dict_ul_int_iterator_t* hi, unsigned long* key, int* value) {
    return htable_iterator_next((htable_iterator_t*) hi, (void**) &key, (void**) &value);
}

void dict_ul_int_iterator_destroy(dict_ul_int_iterator_t* hi) {
    return htable_iterator_destroy((htable_iterator_t*) hi);
}

