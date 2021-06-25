#ifndef BITMAP_H
#define BITMAP_H

#include <limits.h>
#include <stddef.h>

typedef char word_t;
enum
{
    BITS_PER_WORD = sizeof(word_t) * CHAR_BIT
};

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

void
set_bit(word_t *words, size_t n);

void
clear_bit(word_t *words, size_t n);

char
get_bit(const word_t *words, size_t n);

#endif
