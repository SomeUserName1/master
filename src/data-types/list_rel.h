
#ifndef __LIST_REL_H__
#define __LIST_REl_H__

#include "list.h"
#include "../access/records/relationship.h"

typedef struct list_relationship list_relationship_t;

list_relationship_t* create_list_relationship(list_flags_t);
void list_relationship_destroy(list_relationship_t* l);
size_t list_relationship_size(list_relationship_t* l);

int list_relationship_append(list_relationship_t* l, relationship_t* v);
int list_relationship_insert(list_relationship_t* l, relationship_t* v, size_t idx);
int list_relationship_remove(list_relationship_t* l, size_t idx);
int list_relationship_remove_elem(list_relationship_t* l, relationship_t* elem);

int list_relationship_index_of(list_relationship_t* l, relationship_t* v, size_t* idx);
bool list_relationship_contains(list_relationship_t* l, relationship_t* v);

relationship_t* list_relationship_get(list_relationship_t* l, size_t idx);
relationship_t* list_relationship_take(list_relationship_t* l, size_t idx);

int list_relationship_start_bulk_add(list_relationship_t* l);
int list_relationship_end_bulk_add(list_relationship_t* l);
int list_relationship_sort(list_relationship_t* l);

#endif
