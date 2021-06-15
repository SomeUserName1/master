#include "set_ul.h"

#include <stdlib.h>

#include "cbs.h"
#include "htable.h"
#include "set.h"

set_ul_t*
create_set_ul(void)
{
    htable_hash  hash = fnv_hash_ul;
    htable_keq   keq  = unsigned_long_eq;
    htable_cbs_t cbs  = {
        unsigned_long_copy, free, unsigned_long_print, NULL, NULL, NULL, NULL
    };
    return (set_ul_t*)create_set(hash, keq, &cbs);
}

void
set_ul_destroy(set_ul_t* set)
{
    set_destroy((set_t*)set);
}

size_t
set_ul_size(set_ul_t* set)
{
    return set_size((set_t*)set);
}

int
set_ul_insert(set_ul_t* set, unsigned long elem)
{
    return set_insert((set_t*)set, &elem);
}

int
set_ul_remove(set_ul_t* set, unsigned long elem)
{
    return set_remove((set_t*)set, &elem);
}

bool
set_ul_contains(set_ul_t* set, unsigned long elem)
{
    return set_contains((set_t*)set, &elem);
}
