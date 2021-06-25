#include "data-struct/bitmap.h"

#include <stddef.h>

void
set_bit(word_t *words, size_t n)
{
    words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void
clear_bit(word_t *words, size_t n)
{
    words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

char
get_bit(const word_t *words, size_t n)
{
    word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0;
}
