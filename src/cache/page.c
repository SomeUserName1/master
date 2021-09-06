#include "page.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "page_cache.h"
#include "physical_database.h"

page*
page_create(unsigned char* data)
{

    if (!data) {
        printf("page - create: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }
    page* p = calloc(1, sizeof(page));

    if (!p) {
        printf("page - create: Allocating memory failed!\n");
        exit(EXIT_FAILURE);
    }

    p->fk      = invalid;
    p->ft      = invalid_ft;
    p->page_no = ULONG_MAX;
    p->data    = data;

    return p;
}

void
page_destroy(page* p)
{
    if (!p) {
        printf("page - destroy: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (p->pin_count != 0 || p->dirty) {
        printf("page - destroy: page is dirty or pinned!\n");
        exit(EXIT_FAILURE);
    }

    free(p);
}

unsigned long
read_ulong(page* p, size_t offset)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned long) || p->pin_count < 1) {
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
    if (!p || offset > PAGE_SIZE - sizeof(unsigned long) || p->pin_count < 1) {
        printf("page - write ulong: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(p->data + offset, &value, sizeof(unsigned long));

    p->dirty = true;
}

unsigned char
read_uchar(page* p, size_t offset)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned char) || p->pin_count < 1) {
        printf("page - read uchar: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    return p->data[offset];
}

void
write_uchar(page* p, size_t offset, unsigned char value)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned char) || p->pin_count < 1) {
        printf("page - write uchar: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    p->data[offset] = value;

    p->dirty = true;
}

double
read_double(page* p, size_t offset)
{
    if (!p || offset > PAGE_SIZE - sizeof(double) || p->pin_count < 1) {
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
    if (!p || offset > PAGE_SIZE - sizeof(double) || p->pin_count < 1) {
        printf("page - write double: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(p->data + offset, &value, sizeof(double));

    p->dirty = true;
}

void
read_string(page* p, size_t offset, char* buf)
{
    if (!p || offset > PAGE_SIZE - sizeof(char) * MAX_STR_LEN
        || p->pin_count < 1) {
        printf("page - read string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(buf, p->data + offset, sizeof(char) * MAX_STR_LEN);
}

void
write_string(page* p, size_t offset, char* value)
{
    if (!p || offset > PAGE_SIZE - sizeof(char) * MAX_STR_LEN
        || value[MAX_STR_LEN - 1] != '\0' || p->pin_count < 1) {
        printf("page - write string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    char buf[MAX_STR_LEN];
    memset(buf, 0, MAX_STR_LEN);
    strncpy(buf, value, MAX_STR_LEN * sizeof(char));
    memcpy(p->data + offset, buf, MAX_STR_LEN);

    p->dirty = true;
}

void
page_pretty_print(const page* p)
{
    if (!p || p->pin_count < 1) {
        printf("page - write string: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    printf("File Kind: %d, File Type: %d, Page No. %zu, pin count: %u, is "
           "dirty? %s\n",
           p->fk,
           p->ft,
           p->page_no,
           p->pin_count,
           p->dirty ? "true" : "false");
}
