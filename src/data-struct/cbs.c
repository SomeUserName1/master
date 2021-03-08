#include "cbs.h"

#include <stdio.h>
#include <stdlib.h>

#include "../record/node.h"
#include "../record/relationship.h"

size_t
fnv_hash_ul(const void* in, unsigned int seed)
{
    size_t h = seed;
    const unsigned int prime = 0xFDCFB7;
    unsigned long ul = *((unsigned long*)in);

    return (h ^ ul) * prime;
}

bool
unsigned_long_eq(const void* a, const void* b)
{
    return *((unsigned long*)a) == *((unsigned long*)b);
}

void*
unsigned_long_copy(const void* orig)
{
    unsigned long* copy = (unsigned long*)malloc(sizeof(*copy));
    *copy = *((unsigned long*)orig);
    return (void*)copy;
}

void
unsigned_long_print(const void* in)
{
    printf("%lu", *((unsigned long*)in));
}

bool
int_eq(const void* first, const void* second)
{
    return (*((int*)first) == *((int*)second));
}

void*
int_copy(const void* original)
{
    int* copy = (int*)malloc(sizeof(*copy));
    *copy = *((int*)original);
    return copy;
}

void
int_print(const void* in)
{
    printf("%i", *((int*)in));
}

bool
node_eq(const void* first, const void* second)
{
    return node_equals((node_t*)first, (node_t*)second);
}

void
node_print(const void* in)
{
    node_pretty_print((node_t*)in);
}

bool
rel_eq(const void* first, const void* second)
{
    return relationship_equals((relationship_t*)first, (relationship_t*)second);
}

void
rel_print(const void* in)
{
    relationship_pretty_print((relationship_t*)in);
}
