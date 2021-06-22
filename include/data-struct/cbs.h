#ifndef CBS_H
#define CBS_H

#include <stdbool.h>
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
fnv_hash_ul(const unsigned long in, unsigned int seed)
{
    size_t             h     = seed;
    const unsigned int prime = 0xFDCFB7;

    return (h ^ in) * prime;
}

bool
unsigned_long_eq(const unsigned long a, const unsigned long b)
{
    return a == b;
}

void
unsigned_long_print(const unsigned long in)
{
    printf("%lu", in);
}

bool
int_eq(const int first, const int second)
{
    return first == second;
}

void
int_print(const int in)
{
    printf("%i", in);
}

void
node_free(node_t* node)
{
    free(node);
}

void
rel_free(relationship_t* rel)
{
    free(rel);
}

#endif
