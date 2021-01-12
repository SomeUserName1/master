#include "dict_ul.h"

#include <stdlib.h>

#include "htable.h"
#include "cbs.h"
#include "../record/node.h"
#include "../record/relationship.h"

static size_t fnv_hash_ul(const void* in, unsigned int seed) {
    size_t h = seed;
    unsigned int prime = 0xFDCFB7;
    unsigned long ul = *((unsigned long*) in);

    return (h ^ ul) * prime;
}

dict_ul_ul_t* create_dict_ul_ul() {
    htable_hash hash = fnv_hash_ul;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        unsigned_long_print,
        unsigned_long_eq,
        unsigned_long_copy,
        free,
        unsigned_long_print
    };

    return (dict_ul_ul_t*) create_htable(hash, keq, &cbs);
}

int dict_ul_ul_destroy(dict_ul_ul_t* ht) {
    return htable_destroy((htable_t*) ht);
}

size_t dict_ul_ul_size(dict_ul_ul_t* ht) {
    return htable_size((htable_t*) ht);
}

int dict_ul_ul_insert(dict_ul_ul_t* ht, unsigned long key, unsigned long value) {
    unsigned long temp_k = key;
    unsigned long temp_v = value;
    return htable_insert((htable_t*) ht, (void*) &temp_k, (void*) &temp_v);
}

int dict_ul_ul_remove(dict_ul_ul_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return  htable_remove((htable_t*) ht, (void*) &t_key);
}

int dict_ul_ul_get(dict_ul_ul_t* ht, unsigned long key, unsigned long** value) {
    unsigned long t_key = key;
    return htable_get((htable_t*) ht, (void*) &t_key, (void**) value);
}

unsigned long dict_ul_ul_get_direct(dict_ul_ul_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return *((unsigned long*) htable_get_direct((htable_t*) ht, (void*) &t_key));
}

bool dict_ul_ul_contains(dict_ul_ul_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_contains((htable_t*) ht, (void*) &t_key);
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

void dict_ul_ul_print(dict_ul_ul_t* dict) {
    htable_print((htable_t*) dict);
}

dict_ul_int_t* create_dict_ul_int() {
    htable_hash hash = fnv_hash_ul;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        unsigned_long_print,
        int_eq,
        int_copy,
        free,
        int_print
    };

    return (dict_ul_int_t*) create_htable(hash, keq, &cbs);
}

int dict_ul_int_destroy(dict_ul_int_t* ht) {
    return htable_destroy((htable_t*) ht);
}

size_t dict_ul_int_size(dict_ul_int_t* ht) {
    return htable_size((htable_t*) ht);
}

int dict_ul_int_insert(dict_ul_int_t* ht, unsigned long key, int value) {
    unsigned long t_key = key;
    unsigned long t_value = value;
    return htable_insert((htable_t*) ht, (void*) &t_key, (void*) &t_value);
}

int dict_ul_int_remove(dict_ul_int_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_remove((htable_t*) ht, (void*) &t_key);
}

int dict_ul_int_get(dict_ul_int_t* ht, unsigned long key, int** value) {
    unsigned long t_key = key;
    return htable_get((htable_t*) ht, (void*) &t_key, (void**) value);
}

int dict_ul_int_get_direct(dict_ul_int_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return *((int*) htable_get_direct((htable_t*) ht, (void*) &t_key));
}

bool dict_ul_int_contains(dict_ul_int_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_contains((htable_t*) ht, (void*) &t_key);
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

void dict_ul_int_print(dict_ul_int_t* dict) {
    htable_print((htable_t*) dict);
}

dict_ul_node_t* create_dict_ul_node(void) {
    htable_hash hash = fnv_hash_ul;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        unsigned_long_print,
        node_eq,
        NULL,
        free,
        node_print
    };

    return (dict_ul_node_t*) create_htable(hash, keq, &cbs);
}

int dict_ul_node_destroy(dict_ul_node_t* ht) {
    return htable_destroy((htable_t*) ht);
}

size_t dict_ul_node_size(dict_ul_node_t* ht) {
    return htable_size((htable_t*) ht);
}

int dict_ul_node_insert(dict_ul_node_t* ht, unsigned long key, node_t* value) {
    unsigned long t_key = key;
    return htable_insert((htable_t*) ht, (void*) &t_key, (void*) value);
}

int dict_ul_node_remove(dict_ul_node_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_remove((htable_t*) ht, (void*) &t_key);
}

int dict_ul_node_get(dict_ul_node_t* ht, unsigned long key, node_t** value) {
    unsigned long t_key = key;
    return  htable_get((htable_t*) ht, (void*) &t_key, (void**) value);
}

node_t* dict_ul_node_get_direct(dict_ul_node_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return (node_t*) htable_get_direct((htable_t*) ht, (void*) &t_key);
}

bool dict_ul_node_contains(dict_ul_node_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_contains((htable_t*) ht, (void*) &t_key);
}

dict_ul_node_iterator_t* create_dict_ul_node_iterator(dict_ul_node_t* ht) {
    return (dict_ul_node_iterator_t*)
        create_htable_iterator((htable_t*) ht);
}

int dict_ul_node_iterator_next(dict_ul_node_iterator_t* hi, unsigned long* key, node_t** value) {
    return htable_iterator_next((htable_iterator_t*) hi, (void**) &key,
                (void**) value);
}

void dict_ul_node_iterator_destroy(dict_ul_node_iterator_t* hi) {
    return htable_iterator_destroy((htable_iterator_t*) hi);
}

void dict_ul_node_print(dict_ul_node_t* dict) {
    htable_print((htable_t*) dict);
}

dict_ul_rel_t* create_dict_ul_rel(void) {
 htable_hash hash = fnv_hash_ul;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        unsigned_long_print,
        rel_eq,
        NULL, 
        free,
        rel_print
    };

    return (dict_ul_rel_t*) create_htable(hash, keq, &cbs);

}

int dict_ul_rel_destroy(dict_ul_rel_t* ht) {
    return htable_destroy((htable_t*) ht);
}

size_t dict_ul_rel_size(dict_ul_rel_t* ht) {
    return htable_size((htable_t*) ht);
}

int dict_ul_rel_insert(dict_ul_rel_t* ht, unsigned long key, relationship_t* value) {
    unsigned long t_key = key;
    return htable_insert((htable_t*) ht, (void*) &t_key, (void*) value);
}

int dict_ul_rel_remove(dict_ul_rel_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_remove((htable_t*) ht, (void*) &t_key);
}

int dict_ul_rel_get(dict_ul_rel_t* ht, unsigned long key, relationship_t** value) {
    unsigned long t_key = key;
    return htable_get((htable_t*) ht, (void*) &t_key, (void**) value);
}

relationship_t* dict_ul_rel_get_direct(dict_ul_rel_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return (relationship_t*) htable_get_direct((htable_t*) ht, (void*) &t_key);
}

bool dict_ul_rel_contains(dict_ul_rel_t* ht, unsigned long key) {
    unsigned long t_key = key;
    return htable_contains((htable_t*) ht, (void*) &t_key);
}

dict_ul_rel_iterator_t* create_dict_ul_rel_iterator(dict_ul_rel_t* ht) {
    return (dict_ul_rel_iterator_t*) create_htable_iterator((htable_t*) ht);
}

int dict_ul_rel_iterator_next(dict_ul_rel_iterator_t* hi, unsigned long* key, relationship_t** value) {
    return htable_iterator_next((htable_iterator_t*) hi, (void**) &key, (void**) value);
}

void dict_ul_rel_iterator_destroy(dict_ul_rel_iterator_t* hi) {
    return htable_iterator_destroy((htable_iterator_t*) hi);
}

void dict_ul_rel_print(dict_ul_rel_t* dict) {
    htable_print((htable_t*) dict);
}
