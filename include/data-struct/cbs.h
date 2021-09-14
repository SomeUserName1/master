/*
 * @(#)cbs.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef CBS_H
#define CBS_H

#include <stdbool.h>
#include <stddef.h>

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

bool
double_eq(double first, double second);

void
double_print(double in);

#endif
