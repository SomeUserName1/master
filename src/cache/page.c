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
    return (fst->page_no == snd->page_no && fst->frame_no == snd->frame_no);
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

    printf("Frame No. %zu, Page No. %zu, pin count: %u, is dirty? %s\n",
           p->frame_no,
           p->page_no,
           p->pin_count,
           p->dirty ? "true" : "false");
}

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

