#ifndef __RELATIONSHIP_LIST_H__
#define __RELATIONSHIP_LIST_H__

#include <stdbool.h>
#include <stddef.h>

#include "list.h"
#include "../storage/records/relationship.h"

typedef struct relationship_list relationship_list_t;

typedef enum {
    REL_LIST_NONE = 0,
    REL_LIST_ID = 1 << 0,
    REL_LIST_ALL = 1 << 1
} rel_list_flags_t;

relationship_list_t* create_relationship_list(rel_list_flags_t flags);
void destroy_rel_list(relationship_list_t* list);
void list_rel_size(relationship_list_t* list);


int list_rel_append(relationship_list_t* l, void *v);
int list_rel_insert(relationship_list_t* l, void *v, size_t idx);
int list_rel_remove(relationship_list_t* l, size_t idx);
int list_rel_remove_elem(relationship_list_t* l, void* elem);

int list_rel_index_of(relationship_list_t* l, void* v, size_t* idx);
bool list_rel_contains(relationship_list_t* l, void* v);

void* list_rel_get(relationship_list_t* l, size_t idx);
void* list_rel_take(relationship_list_t* l, size_t idx);

int list_rel_start_bulk_add(relationship_list_t* l);
int list_rel_end_bulk_add(relationship_list_t* l);

int list_rel_sort(relationship_list_t* l, list_eq e);
#endif
