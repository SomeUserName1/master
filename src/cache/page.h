#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <stdio.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"

#include "data-struct/htable.h"
#include "data-struct/linked_list.h"

typedef enum
{
    bitmap_page       = 1,
    node_page         = sizeof(node_t),
    relationship_page = sizeof(relationship_t)
} page_type;

typedef struct
{
    page_type     pt;
    size_t        page_no;
    short         slot_size;
    unsigned int  pin_count;
    bool          dirty;
    unsigned char data[PAGE_SIZE / sizeof(unsigned char)];
} page;

page*
create_page(size_t id, size_t slot_size);

bool
page_equals(const page* fst, const page* snd);

size_t
get_free_slot(const page* p);

void
clear_slot(page* p, size_t slot_no);

void
write_slot(page* p);

unsigned char*
read_slot(const page* p, size_t slot_no);

void
page_pretty_print(const page* p);

LINKED_LIST_DECL(linked_list_page, page*);
linked_list_page*
ll_page_create(void);

HTABLE_DECL(dict_ul_page, unsigned long, page*)

dict_ul_page*
d_ul_page_create(void);

#endif
