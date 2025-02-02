/*!
 * \file page.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref page.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz. \
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "page.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "page_cache.h"
#include "physical_database.h"
#include "strace.h"

page*
page_create(unsigned char* data)
{

    if (!data) {
        // LCOV_EXCL_START
        printf("page - create: Invalid Arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    page* p = calloc(1, sizeof(page));

    if (!p) {
        // LCOV_EXCL_START
        printf("page - create: Allocating memory failed!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
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
        // LCOV_EXCL_START
        printf("page - destroy: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    if (p->pin_count != 0 || p->dirty) {
        // LCOV_EXCL_START
        printf("page - destroy: page is dirty or pinned!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    free(p);
}

unsigned long
read_ulong(page* p, size_t offset)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned long) || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - read ulong: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    unsigned long res;
    memcpy(&res, p->data + offset, sizeof(unsigned long));

    return res;
}

void
write_ulong(page* p, size_t offset, unsigned long value)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned long) || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - write ulong: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    memcpy(p->data + offset, &value, sizeof(unsigned long));

    p->dirty = true;
}

unsigned char
read_uchar(page* p, size_t offset)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned char) || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - read uchar: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    return p->data[offset];
}

void
write_uchar(page* p, size_t offset, unsigned char value)
{
    if (!p || offset > PAGE_SIZE - sizeof(unsigned char) || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - write uchar: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    p->data[offset] = value;

    p->dirty = true;
}

double
read_double(page* p, size_t offset)
{
    if (!p || offset > PAGE_SIZE - sizeof(double) || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - read double: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    double res;
    memcpy(&res, p->data + offset, sizeof(double));

    return res;
}

void
write_double(page* p, size_t offset, double value)
{
    if (!p || offset > PAGE_SIZE - sizeof(double) || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - write double: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    memcpy(p->data + offset, &value, sizeof(double));

    p->dirty = true;
}

void
read_string(page* p, size_t offset, char* buf, size_t len)
{
    if (!p || offset > PAGE_SIZE - sizeof(char) * len || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - read string: Invalid arguments!\n");
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    memcpy(buf, p->data + offset, sizeof(char) * len);
}

void
write_string(page* p, size_t offset, char* value, size_t len)
{
    if (!p || offset > PAGE_SIZE - sizeof(char) * len || value[len - 1] != '\0'
        || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - write string: Invalid arguments! %s\n", value);
        print_trace();
        \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    char buf[len];
    memset(buf, 0, len);
    strncpy(buf, value, len * sizeof(char));
    memcpy(p->data + offset, buf, len);

    p->dirty = true;
}

void
page_pretty_print(const page* p)
{
    if (!p || p->pin_count < 1) {
        // LCOV_EXCL_START
        printf("page - write string: Invalid arguments!\n");
        print_trace(); \
exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    printf("File Kind: %d, File Type: %d, Page No. %zu, pin count: %u, is "
           "dirty? %s\n",
           p->fk,
           p->ft,
           p->page_no,
           p->pin_count,
           p->dirty ? "true" : "false");
}
