/*!
 * \file header_page.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref header_page.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "access/header_page.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "page.h"
#include "page_cache.h"
#include "strace.h"

bool
compare_bits(const unsigned char* ar,
             size_t               size,
             unsigned char        mask,
             size_t               offset,
             size_t               n_bits)
{
    if (!ar || offset + n_bits > size || n_bits > CHAR_BIT) {
        // LCOV_EXCL_START
        printf("header page - compare bits: Invalid Arguments!, ar %p, size "
               "%lu, mask %u, offset %lu, n_bits %lu\n",
               ar,
               size,
               mask,
               offset,
               n_bits);
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long start_byte = offset / CHAR_BIT;
    unsigned char bit_offset = offset % CHAR_BIT;

    mask = mask & (UCHAR_MAX >> (CHAR_BIT - n_bits));

    unsigned char result = 0;
    if (bit_offset == 0 || bit_offset + n_bits <= CHAR_BIT) {
        shift_bit_array(
              &mask, CHAR_BIT, -(long)(CHAR_BIT - n_bits) + bit_offset);

        unsigned char data_mask =
              (unsigned char)(UCHAR_MAX << (CHAR_BIT - (n_bits + bit_offset)))
              & (UCHAR_MAX >> bit_offset);

        result = ar[start_byte] & data_mask;
    } else {
        unsigned char upper_mask = UCHAR_MAX >> bit_offset;
        unsigned char lower_mask =
              UCHAR_MAX << (CHAR_BIT - n_bits + CHAR_BIT - bit_offset);

        unsigned char shift_result[2];
        shift_result[0] = (ar[start_byte] & upper_mask);
        shift_result[1] = (ar[start_byte + 1] & lower_mask);

        shift_bit_array(
              shift_result, 2 * CHAR_BIT, CHAR_BIT - bit_offset - n_bits);

        result = shift_result[0];
    }

    return result == mask;
}

/**
 * a negative amount means left, a positive one right.
 */
void
shift_bit_array(unsigned char* ar, size_t size, long n_bits)
{
    if (!ar || (unsigned int)labs(n_bits) > size || labs(n_bits) > CHAR_BIT
        || n_bits > CHAR_BIT) {
        // LCOV_EXCL_START
        printf("page - shift bit array: Invalid Arguments!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (n_bits == 0) {
        return;
    }

    unsigned char carry_mask;
    if (n_bits > 0) {
        carry_mask = UCHAR_MAX >> (CHAR_BIT - n_bits);
    } else {
        carry_mask = UCHAR_MAX << (CHAR_BIT - (labs(n_bits)));
    }

    unsigned char carry = 0;
    unsigned char next_carry;
    size_t        n_bytes = size / CHAR_BIT + (size % CHAR_BIT != 0);

    if (n_bits > 0) {
        for (size_t i = 0; i < n_bytes; ++i) {
            next_carry = ar[i] & carry_mask;

            ar[i] = (carry << (CHAR_BIT - n_bits)) | (ar[i] >> n_bits);

            carry = next_carry;
        }
    } else {
        for (long i = (long)n_bytes - 1; i >= 0; --i) {
            next_carry = (unsigned char)(ar[i] & carry_mask);

            ar[i] = (carry >> (CHAR_BIT - labs(n_bits)))
                    | (ar[i] << labs(n_bits));

            carry = next_carry;
        }
    }
}

/*
 * Concat the first bit array and the second both into the first one. The second
 * array is freed.
 */
unsigned char*
concat_bit_arrays(unsigned char* first,
                  unsigned char* second,
                  size_t         n_bits_fst,
                  size_t         n_bits_snd)
{
    if (!first || !second || n_bits_fst > LONG_MAX || n_bits_snd > LONG_MAX) {
        // LCOV_EXCL_START
        printf("page - concat bit arrays: Invalid Arguemnts!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    size_t result_size = ((n_bits_fst + n_bits_snd) / CHAR_BIT)
                         + ((n_bits_fst + n_bits_snd) % CHAR_BIT != 0);

    size_t first_size = (n_bits_fst / CHAR_BIT) + (n_bits_fst % CHAR_BIT != 0);

    first = realloc(first, result_size * sizeof(unsigned char));

    if (!first) {
        // LCOV_EXCL_START
        printf("page - concat_bit_arrays: Failed to allocate memory!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    for (size_t i = first_size; i < result_size; ++i) {
        first[i] = 0;
    }

    // [XXXX XXXX] concat [0000 00YY] => [0000 00XX] [XXXX XXYY]

    shift_bit_array(first,
                    n_bits_fst,
                    (long)(result_size * CHAR_BIT - n_bits_fst - n_bits_snd));

    /* If the point of concatenation is not byte-aligned, mask the last byte of
     * the first array and the first byte of the second array and concatenate
     * them. using an or */
    if (n_bits_fst % CHAR_BIT != 0) {
        first[first_size - 1] =
              (first[first_size - 1] & (UCHAR_MAX << n_bits_snd))
              | (second[0] & (UCHAR_MAX >> n_bits_snd));
    }

    for (size_t i = 0; i < result_size - first_size; ++i) {
        first[first_size + i] = second[i];
    }

    free(second);

    return first;
}

/*
 * Modifies the first array, such that it carries the MSBs and returns another
 * array with the LSBs
 */
unsigned char**
split_bit_array(unsigned char* ar, size_t size, size_t split_at_bit)
{
    if (!ar
        || (split_at_bit / CHAR_BIT) + (split_at_bit % CHAR_BIT != 0) > size) {
        // LCOV_EXCL_START
        printf("page - split bit array: Invalid Arguments!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t first_size =
          (split_at_bit / CHAR_BIT) + (split_at_bit % CHAR_BIT != 0);

    size_t second_size =
          (size / CHAR_BIT) - first_size + (split_at_bit % CHAR_BIT != 0);

    unsigned char* second = calloc(second_size, sizeof(unsigned char));

    memcpy(second, ar + (split_at_bit / CHAR_BIT), second_size);

    second[0] = second[0] & (UCHAR_MAX >> (split_at_bit % CHAR_BIT));

    shift_bit_array(ar, size, (long)(split_at_bit % CHAR_BIT));

    unsigned char* new_ar = realloc(ar, first_size * sizeof(unsigned char));

    if (!ar) {
        // LCOV_EXCL_START
        printf("page - concat_bit_arrays: Failed to allocate memory!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    } else {
        ar = new_ar;
    }

    unsigned char** result = malloc(2 * sizeof(unsigned char*));
    result[0]              = ar;
    result[1]              = second;

    return result;
}

unsigned char*
read_bits(page_cache*    pc,
          page*          p,
          unsigned short byte_offset_in_page,
          unsigned char  bit_offset_in_byte,
          unsigned long  n_bits,
          bool           log)
{
    if (!p || bit_offset_in_byte > CHAR_BIT - 1
        || byte_offset_in_page > PAGE_SIZE - 1
        || n_bits > PAGE_SIZE * CHAR_BIT - 1) {
        // LCOV_EXCL_START
        printf("header page - read bits: Invalid Arguments!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    bool           split_read = false;
    unsigned char* second_part;
    size_t         n_bits_split_read;
    size_t n_bytes_result = (n_bits / CHAR_BIT) + (n_bits % CHAR_BIT != 0);

    size_t n_bytes_read = ((bit_offset_in_byte + n_bits) / CHAR_BIT)
                          + ((bit_offset_in_byte + n_bits) % CHAR_BIT != 0);

    if (byte_offset_in_page + n_bytes_read > PAGE_SIZE) {
        size_t bits_to_page_boundary =
              (PAGE_SIZE - byte_offset_in_page) * CHAR_BIT
              - (bit_offset_in_byte);
        n_bits_split_read = n_bits - bits_to_page_boundary;

        page* next_page = pin_page(pc, p->page_no + 1, p->fk, p->ft, log);

        second_part = read_bits(pc, next_page, 0, 0, n_bits_split_read, log);

        unpin_page(pc, p->page_no + 1, p->fk, p->ft, log);

        n_bits         = bits_to_page_boundary;
        n_bytes_result = (n_bits / CHAR_BIT) + (n_bits % CHAR_BIT != 0);
        n_bytes_read   = ((bit_offset_in_byte + n_bits) / CHAR_BIT)
                       + ((bit_offset_in_byte + n_bits) % CHAR_BIT != 0);
        split_read = true;
    }

    unsigned char* result = calloc(n_bytes_result, sizeof(unsigned char));

    // Read the bytes that contain the bits of interest
    //
    // Shift the individual bits of the bytes downwards, such that the last bit
    // of interestis the last element in the unsigned char[]
    // e.g. if we want to read 6 bits with an offet of 0,
    // [XXXX XX__] gets [00XX XXXX]

    unsigned long shift =
          n_bytes_read * CHAR_BIT - (n_bits + bit_offset_in_byte);
    unsigned char carry_mask = UCHAR_MAX >> (CHAR_BIT - shift);
    unsigned char carry;
    unsigned char next_carry;

    // Zero the first bit_offset_in_byte bits
    result[0] =
          p->data[byte_offset_in_page] & (UCHAR_MAX >> bit_offset_in_byte);

    // set the carry and shift the first byte
    carry = result[0] & carry_mask;

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
    size_t j = 0;
    if (n_bytes_read > n_bytes_result) {
        result[0] = 0;
        ++j;
    }

    for (size_t i = 1; i < n_bytes_read; ++i) {
        next_carry    = p->data[byte_offset_in_page + i] & carry_mask;
        result[i - j] = (carry << (CHAR_BIT - shift))
                        | (p->data[byte_offset_in_page + i] >> shift);
        carry = next_carry;
    }

    /* If a split read has been performed, concatenate the results.
     * The size of the arrays is converted to be byte aligned (as all neccessary
     * shifts have been done already) and back to bits then
     */

    return split_read
                 ? concat_bit_arrays(result,
                                     second_part,
                                     n_bytes_result * CHAR_BIT,
                                     ((n_bits_split_read / CHAR_BIT)
                                      + (n_bits_split_read % CHAR_BIT != 0))
                                           * CHAR_BIT)
                 : result;
}

void
write_bits(page_cache*    pc,
           page*          p,
           unsigned short byte_offset_in_page,
           unsigned char  bit_offset_in_byte,
           unsigned long  n_bits,
           unsigned char* data,
           bool           log)
{
    if (!p || bit_offset_in_byte > CHAR_BIT - 1
        || byte_offset_in_page > PAGE_SIZE - 1
        || n_bits > PAGE_SIZE * CHAR_BIT - 1 || n_bits < 1 || !data) {
        // LCOV_EXCL_START
        printf("header page - write bits: Invalid Arguments!\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t n_bytes_write = ((bit_offset_in_byte + n_bits) / CHAR_BIT)
                           + ((bit_offset_in_byte + n_bits) % CHAR_BIT != 0);

    unsigned long n_bytes_data = (n_bits / CHAR_BIT) + (n_bits % CHAR_BIT != 0);

    if (byte_offset_in_page + n_bytes_write > PAGE_SIZE) {
        size_t bits_to_page_boundary =
              (PAGE_SIZE - byte_offset_in_page) * CHAR_BIT
              - (bit_offset_in_byte);
        size_t n_bits_split_write = n_bits - bits_to_page_boundary;

        unsigned char** split_data = split_bit_array(
              data, n_bytes_data * CHAR_BIT, n_bits_split_write);

        page* next_page = pin_page(pc, p->page_no + 1, p->fk, p->ft, log);

        write_bits(pc, next_page, 0, 0, n_bits_split_write, split_data[1], log);

        unpin_page(pc, p->page_no + 1, p->fk, p->ft, log);

        data          = split_data[0];
        n_bits        = bits_to_page_boundary;
        n_bytes_write = ((bit_offset_in_byte + n_bits) / CHAR_BIT)
                        + ((bit_offset_in_byte + n_bits) % CHAR_BIT != 0);
        n_bytes_data = (n_bits / CHAR_BIT) + (n_bits % CHAR_BIT != 0);

        free(split_data);
    }

    // Shift data by (n bytes * CHAR_BIT - n_bits) - bit offset to left
    // CHAR_BIT - n_bits give the leading zeros of the data array
    // - bit_offset gives how many zeros need to be removed
    // e.g. with 5 bits and 1 offset
    // [000X XXXX] => [0XXX XX00]
    // or e.g. with 8 bit and 2 offset
    // [XXXX XXXX] => [00XX XXXX] [XX00 0000]

    unsigned char* shifted_data;

    // n_bytes_data is always larger than n_bits obviously
    data[0] = /* NOLINTNEXTLINE */
          data[0] & UCHAR_MAX >> (n_bytes_data * CHAR_BIT - n_bits);

    if (n_bytes_data < n_bytes_write) {
        shifted_data = calloc(n_bytes_write, sizeof(unsigned char));
        memcpy(shifted_data, data, n_bytes_data);
    } else {
        shifted_data = data;
    }

    shift_bit_array(shifted_data,
                    n_bytes_write * CHAR_BIT,
                    -(long)(n_bytes_data * CHAR_BIT - (long)n_bits)
                          + bit_offset_in_byte);

    if (n_bytes_write == 0) {
        // LCOV_EXCL_START
        printf("Unreachable\n");
        print_trace();

        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

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

    if (((n_bits + bit_offset_in_byte) % CHAR_BIT) != 0) {
        // set the last 8 - n_bits to preserve the data on the page
        mask[n_bytes_write - 1] =
              mask[n_bytes_write - 1] /* NOLINTNEXTLINE */
              | UCHAR_MAX >> ((n_bits % CHAR_BIT) + bit_offset_in_byte);
    }

    for (size_t i = 0; i < n_bytes_write; ++i) {
        p->data[byte_offset_in_page + i] =
              (p->data[byte_offset_in_page + i] & mask[i]) | shifted_data[i];
    }

    if (data != shifted_data) {
        free(shifted_data);
    }
    free(data);

    p->dirty = true;
}
