#include "list_l.h"

#include <stdlib.h>

#include "cbs.h"
#include "list.h"

list_l_t*
create_list_l()
{
    list_cbs_t cbs = { unsigned_long_eq, unsigned_long_copy, free };
    list_t*    lst = create_list(&cbs);

    return (list_l_t*)lst;
}

void
list_l_destroy(list_l_t* l)
{
    return list_destroy((list_t*)l);
}

size_t
list_l_size(list_l_t* l)
{
    return list_size((list_t*)l);
}

int
list_l_append(list_l_t* l, long v)
{
    return list_append((list_t*)l, (void*)&v);
}

int
list_l_insert(list_l_t* l, long v, size_t idx)
{
    return list_insert((list_t*)l, (void*)&v, idx);
}

int
list_l_remove(list_l_t* l, size_t idx)
{
    return list_remove((list_t*)l, idx);
}

int
list_l_remove_elem(list_l_t* l, long elem)
{
    return list_remove_elem((list_t*)l, (void*)&elem);
}

int
list_l_index_of(list_l_t* l, long v, size_t* idx)
{
    return list_index_of((list_t*)l, (void*)&v, idx);
}

bool
list_l_contains(list_l_t* l, long v)
{
    return list_contains((list_t*)l, (void*)&v);
}

long
list_l_get(list_l_t* l, size_t idx)
{
    return *((long*)list_get((list_t*)l, idx));
}

long*
list_l_take(list_l_t* l, size_t idx)
{
    return (long*)list_take((list_t*)l, idx);
}
