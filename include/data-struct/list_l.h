#ifndef LIST_L_H
#define LIST_L_H

#include <stdbool.h>
#include <stddef.h>

typedef struct list_l list_l_t;

list_l_t*
create_list_l(void);
void
list_l_destroy(list_l_t* l);
size_t
list_l_size(list_l_t* l);

int
list_l_append(list_l_t* l, long v);
int
list_l_insert(list_l_t* l, long v, size_t idx);
int
list_l_remove(list_l_t* l, size_t idx);
int
list_l_remove_elem(list_l_t* l, long elem);

int
list_l_index_of(list_l_t* l, long v, size_t* idx);
bool
list_l_contains(list_l_t* l, long v);

long
list_l_get(list_l_t* l, size_t idx);
long*
list_l_take(list_l_t* l, size_t idx);

#endif
