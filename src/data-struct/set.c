#include "data-struct/set.h"

#include "data-struct/cbs.h"

SET_IMPL(set_ul, unsigned long, fnv_hash_ul, unsigned_long_eq);
set_ul_cbs s_ul_cbs = { NULL, NULL, unsigned_long_print };

set_ul*
s_ul_create(void)
{
    return set_ul_create(s_ul_cbs);
}

