#include "data-struct/htable.h"
#include "data-struct/cbs.h"

HTABLE_IMPL(dict_ul_ul,
            unsigned long,
            unsigned long,
            fnv_hash_ul,
            unsigned_long_eq);
dict_ul_ul_cbs d_ul_cbs = { NULL, NULL, unsigned_long_print, unsigned_long_eq,
                            NULL, NULL, unsigned_long_print };

dict_ul_ul*
d_ul_ul_create(void)
{
    return dict_ul_ul_create(d_ul_cbs);
}

HTABLE_IMPL(dict_ul_int, unsigned long, int, fnv_hash_ul, unsigned_long_eq);
dict_ul_int_cbs d_int_cbs = { NULL, NULL,     unsigned_long_print, int_eq, NULL,
                              NULL, int_print };

dict_ul_int*
d_ul_int_create(void)
{
    return dict_ul_int_create(d_int_cbs);
}

