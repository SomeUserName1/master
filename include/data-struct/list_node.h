#ifndef LIST_NODE_H
#define LIST_NODE_H

#include <stdbool.h>
#include <stddef.h>

#include "access/node.h"

typedef struct list_node list_node_t;

list_node_t*
create_list_node(void);
void
list_node_destroy(list_node_t* l);
size_t
list_node_size(list_node_t* l);

int
list_node_append(list_node_t* l, node_t* v);
int
list_node_insert(list_node_t* l, node_t* v, size_t idx);
int
list_node_remove(list_node_t* l, size_t idx);
int
list_node_remove_elem(list_node_t* l, node_t* elem);

int
list_node_index_of(list_node_t* l, node_t* v, size_t* idx);
bool
list_node_contains(list_node_t* l, node_t* v);

node_t*
list_node_get(list_node_t* l, size_t idx);
node_t*
list_node_take(list_node_t* l, size_t idx);

#endif
