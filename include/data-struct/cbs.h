#ifndef CBS_H
#define CBS_H

#include "htable.h"
#include <stdbool.h>

bool
ptr_eq(const void* a, const void* b);

unsigned long
fnv_hash_ul(const void* in, unsigned int seed);

bool
unsigned_long_eq(const void* a, const void* b);

void*
unsigned_long_copy(const void* orig);

void
unsigned_long_print(const void* in);

bool
int_eq(const void* first, const void* second);

void*
int_copy(const void* original);

void
int_print(const void* in);

bool
node_eq(const void* first, const void* second);

void
node_print(const void* in);

bool
rel_eq(const void* first, const void* second);
void
rel_print(const void* in);
#endif
