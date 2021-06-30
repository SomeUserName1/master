#ifndef BITMAP_H
#define BITMAP_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <zconf.h>

#define BITMAP_WORD_OFFSET(b) ((b) / CHAR_BIT)
#define BITMAP_BIT_OFFSET(b)  ((b) % CHAR_BIT)
#define BITMAP_N_BYTES(n_bits)                                                 \
    (((n_bits) / CHAR_BIT) + ((n_bits) % CHAR_BIT != 0))

typedef struct
{
    unsigned char* words;
    size_t         n_bits;
} bitmap;

bitmap*
bitmap_create(size_t size);

void
bitmap_destroy(bitmap* b);

void
set_bit(bitmap* b, size_t n);

void
clear_bit(bitmap* b, size_t n);

unsigned char
get_bit(const bitmap* b, size_t n);

bool
all_bits_set(const bitmap* b);

size_t
get_first_unset(const bitmap* b);

#endif
