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

unsigned long
unsigned_long_copy(const unsigned long orig)
{
    return orig;
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

int
int_copy(const int original)
{
    return original;
}

void
int_print(const int in)
{
    printf("%i", in);
}

bool
node_eq(const node_t* first, const node_t* second)
{
    if (!first || !second) {
        printf("cbs - node_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return node_equals(first, second);
}

void
node_print(const node_t* in)
{
    if (!in) {
        printf("cbs - node_print: Invalid Arguments!\n");
        exit(-1);
    }
    node_pretty_print(in);
}

bool
rel_eq(const relationship_t* first, const relationship_t* second)
{
    if (!first || !second) {
        printf("cbs - rel_eq: Invalid Arguments!\n");
        exit(-1);
    }
    return relationship_equals(first, second);
}

void
rel_print(const relationship_t* in)
{
    if (!in) {
        printf("cbs - rel_print: Invalid Arguments!\n");
        exit(-1);
    }
    relationship_pretty_print(in);
}

#endif
