#include "htable.h"

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

HTABLE_IMPL(dict_ul_node,
            unsigned long,
            node_t*,
            fnv_hash_ul,
            unsigned_long_eq);

dict_ul_node_cbs d_node_cbs = {
    NULL, NULL,      unsigned_long_print, node_equals,
    NULL, node_free, node_pretty_print
};

dict_ul_node*
d_ul_node_create(void)
{
    return dict_ul_node_create(d_node_cbs);
}

HTABLE_IMPL(dict_ul_rel,
            unsigned long,
            relationship_t*,
            fnv_hash_ul,
            unsigned_long_eq);
dict_ul_rel_cbs d_rel_cbs = {
    NULL, NULL,     unsigned_long_print,      relationship_equals,
    NULL, rel_free, relationship_pretty_print
};

dict_ul_rel*
d_ul_rel_create(void)
{
    return dict_ul_rel_create(d_rel_cbs);
}

