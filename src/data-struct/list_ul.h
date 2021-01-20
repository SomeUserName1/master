#ifndef LIST_UL_H
#define LIST_UL_H

#include <stdbool.h>
#include <stddef.h>

#include "list.h"

typedef struct list_ul list_ul_t;

list_ul_t* create_list_ul(list_flags_t flags);
void list_ul_destroy(list_ul_t* l);
size_t list_ul_size(list_ul_t* l);

int list_ul_append(list_ul_t* l, unsigned long v);
int list_ul_insert(list_ul_t* l, unsigned long v, size_t idx);
int list_ul_remove(list_ul_t* l, size_t idx);
int list_ul_remove_elem(list_ul_t* l, unsigned long elem);

int list_ul_index_of(list_ul_t* l, unsigned long v, size_t* idx);
bool list_ul_contains(list_ul_t* l, unsigned long v);

unsigned long list_ul_get(list_ul_t* l, size_t idx);
unsigned long list_ul_take(list_ul_t* l, size_t idx);

int list_ul_start_bulk_add(list_ul_t* l);
int list_ul_end_bulk_add(list_ul_t* l);
int list_ul_sort(list_ul_t* l);

#endif
