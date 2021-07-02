#include "data-struct/bitmap.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

bitmap*
bitmap_create(unsigned long size)
{
    bitmap* b = calloc(1, sizeof(bitmap));
    b->n_bits = size;
    b->words  = calloc(BITMAP_N_BYTES(size), sizeof(unsigned char));

    return b;
}

void
bitmap_destroy(bitmap* b)
{
    free(b->words);
    free(b);
}

void
set_bit(bitmap* b, unsigned long n)
{
    if (n >= b->n_bits - 1) {
        printf("bitmap - set bit: Out of Range. tried to set bit %zu, bitmap "
               "size %zu",
               n,
               b->n_bits);
        exit(EXIT_FAILURE);
    }

    b->words[BITMAP_WORD_OFFSET(n)] |= (1 << BITMAP_BIT_OFFSET(n));
}

void
clear_bit(bitmap* b, unsigned long n)
{
    if (n >= b->n_bits - 1) {
        printf("bitmap - clear bit: Out of Range. tried to clear bit %zu, "
               "bitmap "
               "size %zu",
               n,
               b->n_bits);
        exit(EXIT_FAILURE);
    }
    b->words[BITMAP_WORD_OFFSET(n)] &= ~(1 << BITMAP_BIT_OFFSET(n));
}

unsigned char
get_bit(const bitmap* b, unsigned long n)
{
    if (n >= b->n_bits - 1) {
        printf("bitmap - get bit: Out of Range. tried to get bit %zu, bitmap "
               "size %zu",
               n,
               b->n_bits);
        exit(EXIT_FAILURE);
    }

    unsigned char bit =
          b->words[BITMAP_WORD_OFFSET(n)] & (1 << BITMAP_BIT_OFFSET(n));
    return bit != 0;
}

bool
all_bits_set(const bitmap* b)
{
    unsigned char set_bits = 0;

    for (unsigned long i = 0; i < BITMAP_N_BYTES(b->n_bits); ++i) {
        set_bits |= b->words[i];
    }

    return set_bits;
}

unsigned long
get_first_unset(const bitmap* b)
{
    /* Iterate over the bytes */
    for (unsigned long i = 0; i < BITMAP_N_BYTES(b->n_bits); ++i) {
        /* If a byte is not [1111 1111] */
        if (b->words[i] != UCHAR_MAX) {
            /* compute the bit offset from the beginning to the byte */
            unsigned long pos = (i * CHAR_BIT);
            /* iterate over the bits of the byte of interest */
            for (unsigned char j = 0; j < CHAR_BIT; ++j) {
                /* mask the j-th bit and see if it is 0 */
                if (!(b->words[i] & (1 << j))) {
                    /* add the within byte offset to the beginning to byte
                     * offset */
                    return pos + j;
                }
            }
        }
    }
    printf("bitmap - get first unset: No unset bit found!\n");
    exit(EXIT_FAILURE);
}

