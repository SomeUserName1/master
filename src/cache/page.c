#include "page.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"

#define DOUBLE_SIGN_SHIFT           (7)
#define DOUBLE_EXP_N_BYTES          (2)
#define DOUBLE_EXP_BIAS             (1023)
#define DOUBLE_EXP_FIRST_BYTE       (1111111)
#define DOUBLE_EXP_SECOND_BYTE      (11110000)
#define DOUBLE_EXP_EXTRACT_LOWER    (1111)
#define DOUBLE_EXP_EXTRACT_UPPER    (111)
#define DOUBLE_MANTISSA_SECOND_BYTE (1111)
#define DOUBLE_MANTISSA_N_BYTES     (7)
// TODO binary converters

page*
page_create(size_t page_no, size_t frame_no)
{
    page* p = calloc(1, sizeof(page));

    if (!p) {
        printf("page - create: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    p->frame_no  = frame_no;
    p->page_no   = page_no;
    p->pin_count = 0;
    p->dirty     = false;
    p->data      = calloc(1, PAGE_SIZE);

    if (!p->data) {
        printf("page - create: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

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
    return (fst->page_no == snd->page_no && fst->frame_no == snd->frame_no);
}

unsigned long
read_ulong(page* p, size_t offset)
{
    if (!p) {
        printf("page - read ulong: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long res = 0;

    for (size_t i = 0; i < sizeof(unsigned long); ++i) {
        res += p->data[offset + i] << ((sizeof(unsigned long) - i) * CHAR_BIT);
    }

    return res;
}

void
write_ulong(page* p, size_t offset, unsigned long value)
{
    if (!p) {
        printf("page - write ulong: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < sizeof(unsigned long); ++i) {
        p->data[offset + i] = value >> ((sizeof(unsigned long) - i) * CHAR_BIT);
    }
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
    memcpy(p->data + offset, &value, sizeof(value));
}

// TODO continue here

const char*
read_string(page* p, size_t offset)
{
    if (!p) {
        printf("page - read string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    char* res = calloc(MAX_STR_LEN, sizeof(char));

    int ret = sscanf(p->data + offset, "%s", res);

    if (ret != 1) {
        printf("page - string double: ");
        if (ret == EOF) {
            printf("%s\n", strerror(errno));
        } else {
            printf("couldn't read a value\n");
        }
        exit(EXIT_FAILURE);
    }

    return res;
}

void
write_string(page* p, size_t offset, char* value)
{
    if (!p) {
        printf("page - write string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    int ret = sprintf(p->data + offset, "%s", value);

    if (ret != sizeof(double)) {
        printf("page - write string: ");
        if (ret < 0) {
            printf("%s\n", strerror(errno));
        } else {
            printf("couldn't write the neccesarry amount of bytes\n");
        }
        exit(EXIT_FAILURE);
    }
}

void
page_pretty_print(const page* p)
{}

HTABLE_IMPL(dict_ul_page, unsigned long, page*, fnv_hash_ul, unsigned_long_eq);
dict_ul_page_cbs d_page_cbs = { NULL, NULL, unsigned_long_print, page_equals,
                                NULL, NULL, page_pretty_print };

dict_ul_page*
d_ul_page_create(void)
{
    return dict_ul_page_create(d_page_cbs);
}

LINKED_LIST_IMPL(linked_list_page, page*);
linked_list_page_cbs ll_page_cbs = { page_equals, NULL, NULL };

linked_list_page*
ll_page_create(void)
{
    return linked_list_page_create(ll_page_cbs);
}

