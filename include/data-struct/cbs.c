#include "cbs.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"

bool
ptr_eq(const void* a, const void* b)
{
    if (!a || !b) {
        printf("cbs - prt_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return a == b;
}

size_t
fnv_hash_ul(const void* in, unsigned int seed)
{
    size_t             h     = seed;
    const unsigned int prime = 0xFDCFB7;
    unsigned long      ul    = *((unsigned long*)in);

    return (h ^ ul) * prime;
}

bool
unsigned_long_eq(const void* a, const void* b)
{
    if (!a || !b) {
        printf("cbs - ul_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return *((unsigned long*)a) == *((unsigned long*)b);
}

void*
unsigned_long_copy(const void* orig)
{
    if (!orig) {
        printf("cbs - ul_copy: Invalid Arguments!\n");
        exit(-1);
    }
    unsigned long* copy = (unsigned long*)malloc(sizeof(*copy));
    *copy               = *((unsigned long*)orig);
    return (void*)copy;
}

void
unsigned_long_print(const void* in)
{
    if (!in) {
        printf("cbs - ul_print: Invalid Arguments!\n");
        exit(-1);
    }
    printf("%lu", *((unsigned long*)in));
}

bool
int_eq(const void* first, const void* second)
{
    if (!first || !second) {
        printf("cbs - int_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return (*((int*)first) == *((int*)second));
}

void*
int_copy(const void* original)
{
    if (!original) {
        printf("cbs - int_copy: Invalid Arguments!\n");
        exit(-1);
    }
    int* copy = (int*)malloc(sizeof(*copy));
    *copy     = *((int*)original);
    return copy;
}

void
int_print(const void* in)
{
    if (!in) {
        printf("cbs - int_print: Invalid Arguments!\n");
        exit(-1);
    }
    printf("%i", *((int*)in));
}

bool
node_eq(const void* first, const void* second)
{
    if (!first || !second) {
        printf("cbs - node_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return node_equals((node_t*)first, (node_t*)second);
}

void
node_print(const void* in)
{
    if (!in) {
        printf("cbs - node_print: Invalid Arguments!\n");
        exit(-1);
    }
    node_pretty_print((node_t*)in);
}

bool
rel_eq(const void* first, const void* second)
{
    if (!first || !second) {
        printf("cbs - rel_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return relationship_equals((relationship_t*)first, (relationship_t*)second);
}

void
rel_print(const void* in)
{
    if (!in) {
        printf("cbs - rel_print: Invalid Arguments!\n");
        exit(-1);
    }
    relationship_pretty_print((relationship_t*)in);
}
