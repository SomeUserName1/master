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
