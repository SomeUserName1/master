#ifndef CBS_H
#define CBS_H

#include <stdbool.h>
#include <stddef.h>

#include "access/node.h"
#include "access/relationship.h"

bool
ptr_eq(const void* a, const void* b);

size_t
fnv_hash_ul(unsigned long in, unsigned int seed);

bool
unsigned_long_eq(unsigned long a, unsigned long b);

void
unsigned_long_print(unsigned long in);

int
ul_cmp(const void* a, const void* b);

bool
int_eq(int first, int second);

void
int_print(int in);

bool
long_eq(long first, long second);

void
long_print(long in);

void
node_free(node_t* node);

void
rel_free(relationship_t* rel);

#endif
