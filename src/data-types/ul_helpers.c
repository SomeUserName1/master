#include "ul_helpers.h"

#include <stdlib.h>

bool unsigned_long_eq(const void* a, const void* b) {
    return *((unsigned long*) a) == *((unsigned long*) b);
}

void* unsigned_long_copy(const void* orig) {
    unsigned long* copy = (unsigned long*) malloc(sizeof(*copy));
    *copy = *((unsigned long*) orig);
    return (void*) copy;
}
