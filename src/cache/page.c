#include "page.h"

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

void
read_string(page* p, size_t offset, char* buf)
{
    if (!p) {
        printf("page - read string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(buf, p->data + offset, sizeof(char) * MAX_STR_LEN);
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

