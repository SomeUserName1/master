#include "list_node.h"

#include "../record/node.h"
#include "cbs.h"
#include "list.h"

list_node_t*
create_list_node()
{
    list_cbs_t cbs = { node_eq, NULL, NULL };
    list_t*    lst = create_list(&cbs);

    return (list_node_t*)lst;
}

void
list_node_destroy(list_node_t* l)
{
    return list_destroy((list_t*)l);
}

size_t
list_node_size(list_node_t* l)
{
    return list_size((list_t*)l);
}

int
list_node_append(list_node_t* l, node_t* v)
{
    return list_append((list_t*)l, (void*)v);
}

int
list_node_insert(list_node_t* l, node_t* v, size_t idx)
{
    return list_insert((list_t*)l, (void*)v, idx);
}

int
list_node_remove(list_node_t* l, size_t idx)
{
    return list_remove((list_t*)l, idx);
}

int
list_node_remove_elem(list_node_t* l, node_t* elem)
{
    return list_remove_elem((list_t*)l, (void*)elem);
}

int
list_node_index_of(list_node_t* l, node_t* v, size_t* idx)
{
    return list_index_of((list_t*)l, (void*)v, idx);
}

bool
list_node_contains(list_node_t* l, node_t* v)
{
    return list_contains((list_t*)l, (void*)v);
}

node_t*
list_node_get(list_node_t* l, size_t idx)
{
    return (node_t*)list_get((list_t*)l, idx);
}

node_t*
list_node_take(list_node_t* l, size_t idx)
{
    return (node_t*)list_take((list_t*)l, idx);
}
