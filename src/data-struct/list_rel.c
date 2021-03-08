#include "list_rel.h"

#include "cbs.h"

#include "../record/relationship.h"
#include "list.h"

list_relationship_t*
create_list_relationship()
{
    list_cbs_t cbs = { rel_eq, NULL, NULL };
    list_t* lst = create_list(&cbs);

    return (list_relationship_t*)lst;
}

void
list_relationship_destroy(list_relationship_t* l)
{
    return list_destroy((list_t*)l);
}

size_t
list_relationship_size(list_relationship_t* l)
{
    return list_size((list_t*)l);
}

int
list_relationship_append(list_relationship_t* l, relationship_t* v)
{
    return list_append((list_t*)l, (void*)v);
}

int
list_relationship_insert(list_relationship_t* l, relationship_t* v, size_t idx)
{
    return list_insert((list_t*)l, (void*)v, idx);
}

int
list_relationship_remove(list_relationship_t* l, size_t idx)
{
    return list_remove((list_t*)l, idx);
}

int
list_relationship_remove_elem(list_relationship_t* l, relationship_t* elem)
{
    return list_remove_elem((list_t*)l, (void*)elem);
}

int
list_relationship_index_of(list_relationship_t* l,
                           relationship_t* v,
                           size_t* idx)
{
    return list_index_of((list_t*)l, (void*)v, idx);
}

bool
list_relationship_contains(list_relationship_t* l, relationship_t* v)
{
    return list_contains((list_t*)l, (void*)v);
}

relationship_t*
list_relationship_get(list_relationship_t* l, size_t idx)
{
    return (relationship_t*)list_get((list_t*)l, idx);
}

relationship_t*
list_relationship_take(list_relationship_t* l, size_t idx)
{
    return (relationship_t*)list_take((list_t*)l, idx);
}
