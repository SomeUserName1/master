#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <stdio.h>

#include "data-struct/htable.h"
#include "data-struct/linked_list.h"

typedef struct
{
    size_t         frame_no;
    size_t         page_no;
    unsigned int   pin_count;
    bool           dirty;
    unsigned char* data;
} page;

page*
page_create(size_t page_no, size_t frame_no);

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

const char*
read_string(page* p, size_t offset);

void
write_string(page* p, size_t offset);

void
page_pretty_print(const page* p);

LINKED_LIST_DECL(linked_list_page, page*);
linked_list_page*
ll_page_create(void);

HTABLE_DECL(dict_ul_page, unsigned long, page*)

dict_ul_page*
d_ul_page_create(void);

#endif
