#include "page.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

page*
page_create(size_t page_no, unsigned char* data)
{
    page* p = calloc(1, sizeof(page));

    if (!p || !p->data) {
        printf("page - create: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    p->page_no   = page_no;
    p->pin_count = 0;
    p->dirty     = false;
    p->data      = data;

    return p;
}

void
page_destroy(page* p)
{
    if (!p) {
        printf("page - destroy: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (p->dirty || p->pin_count != 0) {
        printf("page - destroy: page is dirty or pinned!\n");
        exit(EXIT_FAILURE);
    }

    free(p->data);
    free(p);
}

bool
page_equals(const page* fst, const page* snd)
{
    if (!fst || !snd) {
        printf("page - equals: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }
    return (fst->ft == snd->ft && fst->pin_count == snd->pin_count
            && fst->dirty == snd->dirty && fst->page_no == snd->page_no);
}

unsigned long
read_ulong(page* p, size_t offset)
{
    if (!p) {
        printf("page - read ulong: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long res;
    memcpy(&res, p->data + offset, sizeof(unsigned long));

    return res;
}

void
write_ulong(page* p, size_t offset, unsigned long value)
{
    if (!p) {
        printf("page - write ulong: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(p->data + offset, &value, sizeof(unsigned long));
}

unsigned char
read_uchar(page* p, size_t offset)
{
    if (!p) {
        printf("page - read uchar: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    return p->data[offset];
}

void
write_uchar(page* p, size_t offset, unsigned char value)
{
    if (!p) {
        printf("page - write uchar: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    p->data[offset] = value;
}

/**
 * a negative amount means left, a positive one right.
 */
void
shift_bit_array(unsigned char* ar, size_t size, long n_bits)
{
    if (!ar || (unsigned int)labs(n_bits * CHAR_BIT) > size) {
        printf("page - shift bit array: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (n_bits == 0) {
        return;
    }

    unsigned char carry_mask;
    if (n_bits > 0) {
        carry_mask = UCHAR_MAX >> (CHAR_BIT - n_bits);
    } else {

        carry_mask = UCHAR_MAX << (CHAR_BIT + n_bits);
    }

    unsigned char carry      = 0;
    unsigned char next_carry = 0;

    for (size_t i = 0; i < size; ++i) {
        next_carry = ar[i] & carry_mask;

        if (n_bits > 0) {
            ar[i] = (carry << (CHAR_BIT - n_bits)) | (ar[i] >> n_bits);
        } else {
            ar[i] = (carry >> (CHAR_BIT - n_bits)) | (ar[i] << n_bits);
        }

        next_carry = carry;
    }
}

/*
 * Concat the first bit array and the second both into the first one. The second
 * array is freed.
 */
void
concat_bit_arrays(unsigned char* first,
                  unsigned char* second,
                  size_t         n_bist_fst,
                  size_t         n_bits_snd)
{
    if (!first || !second || n_bist_fst > LONG_MAX || n_bits_snd > LONG_MAX) {
        printf("page - concat bit arrays: Invalid Arguemnts!\n");
        exit(EXIT_FAILURE);
    }
    size_t result_size = ((n_bist_fst + n_bits_snd) / CHAR_BIT)
                         + ((n_bist_fst + n_bits_snd) % CHAR_BIT != 0);

    size_t first_size = (n_bist_fst / CHAR_BIT) + (n_bist_fst % CHAR_BIT != 0);

    first = realloc(first, result_size * sizeof(unsigned char));

    if (!first) {
        free(first);
        printf("page - concat_bit_arrays: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = first_size; i < result_size; ++i) {
        first[i] = 0;
    }

    // [XXXX XXXX] concat [0000 00YY] => [0000 00XX] [XXXX XXYY]

    shift_bit_array(first, n_bist_fst, (long)n_bits_snd);

    for (size_t i = 0; i < result_size; ++i) {
        first[i] = first[i] | second[i];
    }

    free(second);
}

/*
 * Modifies the first array, such that it carries the MSBs and returns another
 * array with the LSBs
 */
unsigned char*
split_bit_array(unsigned char* ar, size_t size, size_t split_at_bit)
{
    if (!ar
        || (split_at_bit / CHAR_BIT) + (split_at_bit % CHAR_BIT != 0) > size) {
        printf("page - split bit array: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t second_size =
          size - (split_at_bit / CHAR_BIT) + (split_at_bit % CHAR_BIT != 0);

    size_t first_size =
          (split_at_bit / CHAR_BIT) + (split_at_bit % CHAR_BIT != 0);

    unsigned char* second = calloc(second_size, sizeof(unsigned char));

    memcpy(second,
           ar + (split_at_bit / CHAR_BIT) + (split_at_bit % CHAR_BIT != 0),
           second_size);
    second[0] =
          second[0] | (UCHAR_MAX >> (CHAR_BIT - (split_at_bit % CHAR_BIT)));

    shift_bit_array(ar, size, -((long)split_at_bit % CHAR_BIT));

    ar = realloc(ar, first_size * sizeof(unsigned char));
    if (!ar) {
        free(ar);
        printf("page - concat_bit_arrays: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    return second;
}

unsigned char*
read_bits(page*          p,
          unsigned short byte_offset_in_page,
          unsigned char  bit_offset_in_byte,
          unsigned short n_bits)
{
    if (!p || bit_offset_in_byte > CHAR_BIT - 1
        || byte_offset_in_page > PAGE_SIZE - 1
        || n_bits > PAGE_SIZE * CHAR_BIT - 1) {
        printf("page - read bits: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t n_bytes_result = (n_bits / CHAR_BIT) + (n_bits % CHAR_BIT != 0);
    unsigned char* result = calloc(n_bytes_result, sizeof(unsigned char));

    size_t n_bytes_read =
          bit_offset_in_byte + n_bits < n_bytes_result * CHAR_BIT
                ? n_bytes_result
                : n_bytes_result + 1;

    // Read the bytes that contain the bits of interest
    //
    // Shift the individual bits of the bytes downwards, such that the last bit
    // of interestis the last element in the unsigned char[]
    // e.g. if we want to read 6 bits with an offet of 0,
    // [XXXX XX__] gets [00XX XXXX]

    unsigned char shift =
          n_bytes_read * CHAR_BIT - (n_bits + bit_offset_in_byte);
    unsigned char carry_mask = UCHAR_MAX >> (CHAR_BIT - shift);
    unsigned char carry;
    unsigned char next_carry;
    size_t        j = 0;

    // Zero the first bit_offset_in_byte bits
    result[j] =
          p->data[byte_offset_in_page] & (UCHAR_MAX >> bit_offset_in_byte);

    // set the carry and shift the first byte
    carry = result[j] & carry_mask;

    // The first byte needs to be considered iff the amount of
    // bytes the result has and that needs to be read from the page is the same.
    //
    // Otherwise the first byte does not contain bits of interest anymore.
    //
    // e.g. We want to read 7 bit with an offset of 2:
    //
    // [__XX XXXX] [X___ ____] will be shifted 16 - (7 + 2) = 7 places
    // first byte: [____ ____], carry: [__XX XXXX], result [_XXX XXXX]
    //
    if (n_bytes_read == n_bytes_result) {
        result[j] = result[j] >> shift;
        ++j;
    }

    for (size_t i = 1; i < n_bytes_read; ++i) {
        next_carry    = p->data[byte_offset_in_page + i] & carry_mask;
        result[i + j] = (carry << (CHAR_BIT - shift))
                        | (p->data[byte_offset_in_page + i] >> shift);
        next_carry = carry;
    }

    return result;
}

void
write_bits(page*          p,
           unsigned short byte_offset_in_page,
           unsigned char  bit_offset_in_byte,
           unsigned short n_bits,
           unsigned char* data)
{
    if (!p || bit_offset_in_byte > CHAR_BIT - 1
        || byte_offset_in_page > PAGE_SIZE - 1
        || n_bits > PAGE_SIZE * CHAR_BIT - 1 || !data) {
        printf("page - read bits: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t n_bytes_write = ((bit_offset_in_byte + n_bits) / CHAR_BIT)
                           + ((bit_offset_in_byte + n_bits) % CHAR_BIT != 0);

    // Shift data by (CHAR_BIT - n_bits) - bit offset to left
    // CHAR_BIT - n_bits give the leading zeros of the data array
    // - bit_offset gives how many zeros need to be removed
    // e.g. with 5 bits and 1 offset
    // [000X XXXX] => [0XXX XX00]
    // or e.g. with 8 bit and 2 offset
    // [XXXX XXXX] => [00XX XXXX] [XX00 0000]

    // FIXME what if result is longer than before
    // e.g. 8 bit shifted 2 to the right needs another byte => realloc
    shift_bit_array(
          data, n_bytes_write, (CHAR_BIT - n_bits) - bit_offset_in_byte);

    // Write the data to the page buffer using a mask
    // Current page b.    mask      shifted input   resulting page byte
    // ([YYYY YYYY] & [1000 0011]) | [0XXX XX00] => [YXXX XXYY]

    // Mask what is currently on the page
    unsigned char mask[n_bytes_write];

    // Set all bits to 0
    for (size_t i = 0; i < n_bytes_write; ++i) {
        mask[i] = 0;
    }

    // set the first bit_offset bits to preserve the data on the page
    mask[0] = mask[0] | UCHAR_MAX << (CHAR_BIT - bit_offset_in_byte);

    // set the last 8 - offset - n_bits to preserve the data on the page
    mask[n_bytes_write - 1] =
          mask[n_bytes_write - 1] | UCHAR_MAX >> (n_bits % CHAR_BIT);

    // FIXME here
    for (size_t i = 0; i < n_bytes_write; ++i) {
        p->data[byte_offset_in_page + i] =
              (p->data[byte_offset_in_page + i] & mask[i]) | data[i];
    }
}

double
read_double(page* p, size_t offset)
{
    if (!p) {
        printf("page - read double: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    double res;
    memcpy(&res, p->data + offset, sizeof(double));

    return res;
}

void
write_double(page* p, size_t offset, double value)
{
    if (!p) {
        printf("page - write double: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(p->data + offset, &value, sizeof(double));
}

char*
read_string(page* p, size_t offset)
{
    if (!p) {
        printf("page - read string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    char* res = calloc(MAX_STR_LEN, sizeof(char));

    memcpy(res, p->data + offset, sizeof(char) * MAX_STR_LEN);

    return res;
}

void
write_string(page* p, size_t offset, char* value)
{
    if (!p) {
        printf("page - write string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(p->data + offset, value, MAX_STR_LEN * sizeof(char));
}

void
page_pretty_print(const page* p)
{
    if (!p) {
        printf("page - write string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    printf("File Type: %d, Page No. %zu, pin count: %u, is dirty? %s\n",
           p->ft,
           p->page_no,
           p->pin_count,
           p->dirty ? "true" : "false");
}

