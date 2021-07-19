#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <stdio.h>

#include "physical_database.h"

typedef struct
{
    file_type      ft;
    size_t         page_no;
    unsigned int   pin_count;
    bool           dirty;
    unsigned char* data;
} page;

page*
page_create(size_t page_no, unsigned char* data);

void
page_destroy(page* p);

bool
page_equals(const page* fst, const page* snd);

unsigned long
read_ulong(page* p, size_t offset);

void
write_ulong(page* p, size_t offset, unsigned long value);

unsigned char
read_uchar(page* p, size_t offset);

void
write_uchar(page* p, size_t offset, unsigned char value);

void
shift_bit_array(unsigned char* ar, size_t size, long n_bits);

/*
 * Concat the first bit array and the second both into the first one. The second
 * array is freed.
 */
void
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
read_bits(page*          p,
          unsigned short byte_offset_in_page,
          unsigned char  bit_offset_in_byte,
          unsigned short n_bits);

void
write_bits(page*          p,
           unsigned short byte_offset_in_page,
           unsigned char  bit_offset_in_byte,
           unsigned short n_bits,
           unsigned char* data);

double
read_double(page* p, size_t offset);

void
write_double(page* p, size_t offset, double value);

char*
read_string(page* p, size_t offset);

void
write_string(page* p, size_t offset, char* value);

void
page_pretty_print(const page* p);

#endif
