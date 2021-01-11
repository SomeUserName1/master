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
        unsigned_long_eq,
        unsigned_long_copy,
        free
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
    htable_hash hash = fnv_hash_ul;
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

size_t dict_ul_int_size(dict_ul_int_t* ht) {
    return htable_size((htable_t*) ht);
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


dict_ul_node_t* create_dict_ul_node(void) {
    htable_hash hash = fnv_hash_ul;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        node_eq,
        node_cpy,
        free
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
    return htable_insert((htable_t*) ht, (void*) &key, (void*) value);
}

int dict_ul_node_remove(dict_ul_node_t* ht, unsigned long key) {
    return htable_remove((htable_t*) ht, (void*) &key);
}

int dict_ul_node_get(dict_ul_node_t* ht, unsigned long key, node_t** value) {
    return  htable_get((htable_t*) ht, (void*) &key, (void**) value);
}

node_t* dict_ul_node_get_direct(dict_ul_node_t* ht, unsigned long key) {
    return (node_t*) htable_get_direct((htable_t*) ht, (void*) &key);
}

bool dict_ul_node_contains(dict_ul_node_t* ht, unsigned long key) {
    return htable_contains((htable_t*) ht, (void*) &key);
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


dict_ul_rel_t* create_dict_ul_rel(void) {
 htable_hash hash = fnv_hash_ul;
    htable_keq keq = unsigned_long_eq;
    htable_cbs_t cbs = {
        unsigned_long_copy,
        free,
        rel_eq,
        rel_copy,
        free
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
    return htable_insert((htable_t*) ht, (void*) &key, (void*) value);
}

int dict_ul_rel_remove(dict_ul_rel_t* ht, unsigned long key) {
    return htable_remove((htable_t*) ht, (void*) &key);
}

int dict_ul_rel_get(dict_ul_rel_t* ht, unsigned long key, relationship_t** value) {
    return htable_get((htable_t*) ht, (void*) &key, (void**) value);
}

relationship_t* dict_ul_rel_get_direct(dict_ul_rel_t* ht, unsigned long key) {
    return (relationship_t*) htable_get_direct((htable_t*) ht, (void*) &key);
}

bool dict_ul_rel_contains(dict_ul_rel_t* ht, unsigned long key) {
    return htable_contains((htable_t*) ht, (void*) &key);
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



