#include "set.h"
#include "htable.h"

set_t*
create_set(htable_hash hash, htable_keq keq, htable_cbs_t* cbs)
{
    return (set_t*)create_htable(hash, keq, cbs);
}

void
set_destroy(set_t* set)
{
    htable_destroy((htable_t*)set);
}

int
set_insert(set_t* set, void* elem)
{
    return htable_insert((htable_t*)set, elem, NULL);
}

int
set_remove(set_t* set, void* elem)
{
    return htable_remove((htable_t*)set, elem);
}

bool
set_contains(set_t* set, void* elem)
{
    return htable_contains((htable_t*)set, elem);
}
