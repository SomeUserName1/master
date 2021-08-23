#ifndef HEADER_PAGE_H
#define HEADER_PAGE_H

#include <stddef.h>

#include "page.h"
#include "page_cache.h"

bool
compare_bits(const unsigned char* ar,
             size_t               size,
             unsigned char        mask,
             size_t               offset);

void
shift_bit_array(unsigned char* ar, size_t size, long n_bits);

/*
 * Concat the first bit array and the second both into the first one. The second
 * array is freed.
 */
unsigned char*
concat_bit_arrays(unsigned char* first,
                  unsigned char* second,
                  size_t         n_bist_fst,
                  size_t         n_bits_snd);

/*
 * Modifies the first array, such that it carries the MSBs and returns another
 * array with the LSBs
 */
unsigned char**
split_bit_array(unsigned char* ar, size_t size, size_t split_at_bit);

unsigned char*
read_bits(page_cache*    pc,
          page*          p,
          unsigned short byte_offset_in_page,
          unsigned char  bit_offset_in_byte,
          unsigned short n_bits);

void
write_bits(page_cache*    pc,
           page*          p,
           unsigned short byte_offset_in_page,
           unsigned char  bit_offset_in_byte,
           unsigned short n_bits,
           unsigned char* data);

#endif
