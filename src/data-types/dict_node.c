#include "dict_node.h"

#include "../storage/records/node.h"
#include "../storage/records/relationship.h"

static unsigned int fnv_hash_node(const void* in, unsigned int seed) {
    unsigned int h = seed;
    unsigned int prime = 0xFDCFB7;
    node_t* node = (node_t*) in;

    h = (h ^ node->id) * prime;
    h = (h ^ node->first_relationship) * prime;

    return h;
}

static bool node_eq(const void* first, const void* second) {
    return node_equals((node_t*) first, (node_t*) second);
}

static bool rel_eq(const void* first, const void* second) {
    return relationship_equals(
            (relationship_t*) first,
            (relationship_t*) second);
}

static bool int_eq(const void* first, const void* second) {
    return (*((int*) first) == *((int*) second));
}

static void* n_copy(void* original) {
    return (void*) node_copy((node_t*) original);
}

static void* rel_copy(void* original) {
    return (void*) relationship_copy((relationship_t*) original);
}

static void* int_copy(void* original) {
    int* copy = malloc(sizeof(*copy));
    *copy = *((int*)original);
    return copy;
}

dict_node_rel_t* create_dict_node_rel() {
    htable_hash hash = fnv_hash_node;
    htable_keq keq = node_eq;
    htable_cbs_t cbs = {
        n_copy,
        free,
        int_eq,
        rel_copy,
        free
    };

    return (dict_node_rel_t*) create_htable(hash, keq, &cbs);
}

dict_node_int_t* create_dict_node_int() {
    htable_hash hash = fnv_hash_node;
    htable_keq keq = node_eq;
    htable_cbs_t cbs = {
        n_copy,
        free,
        int_eq,
        int_copy,
        free
    };

    return (dict_node_int_t*) create_htable(hash, keq, &cbs);
}

