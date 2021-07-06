#ifndef BITMAP_H
#define BITMAP_H

#include <limits.h>
#include <stdbool.h>

#define BITMAP_WORD_OFFSET(b) ((b) / CHAR_BIT)
#define BITMAP_BIT_OFFSET(b)  ((b) % CHAR_BIT)
#define BITMAP_N_BYTES(n_bits)                                                 \
    (((n_bits) / CHAR_BIT) + ((n_bits) % CHAR_BIT != 0))

typedef struct
{
    unsigned long  n_bits;
    unsigned char* words;
} bitmap;

bitmap*
bitmap_create(unsigned long size);

void
bitmap_destroy(bitmap* b);

void
set_bit(bitmap* b, unsigned long n);

void
clear_bit(bitmap* b, unsigned long n);

unsigned char
get_bit(const bitmap* b, unsigned long n);

bool
all_bits_set(const bitmap* b);

unsigned long
get_first_unset(const bitmap* b);

#endif
