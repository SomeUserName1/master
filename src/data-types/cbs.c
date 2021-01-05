#include "cbs.h"

#include <stdlib.h>

#include "../access/records/node.h"
#include "../access/records/relationship.h"

bool unsigned_long_eq(const void* a, const void* b) {
    return *((unsigned long*) a) == *((unsigned long*) b);
}

void* unsigned_long_copy(const void* orig) {
    unsigned long* copy = (unsigned long*) malloc(sizeof(*copy));
    *copy = *((unsigned long*) orig);
    return (void*) copy;
}

bool int_eq(const void* first, const void* second) {
    return (*((int*) first) == *((int*) second));
}

void* int_copy(const void* original) {
    int* copy = (int*) malloc(sizeof(*copy));
    *copy = *((int*)original);
    return copy;
}

bool node_eq(const void* first, const void* second) {
    return node_equals((node_t*) first, (node_t*) second);
}

void* node_cpy(const void* original) {
    return node_copy((node_t*) original);
}

bool rel_eq(const void* first, const void* second) {
    return relationship_equals(
            (relationship_t*) first,
            (relationship_t*) second
            );
}

void* rel_copy(const void* original) {
    return relationship_copy((relationship_t*) original);
}

