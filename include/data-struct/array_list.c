#include "array_list.h"

#include "cbs.h"

ARRAY_LIST_IMPL(array_list_ul, unsigned long);
array_list_ul_cbs al_ul_cbs = { unsigned_long_eq, NULL, NULL };

inline array_list_ul*
al_ul_create(void)
{
    return array_list_ul_create(al_ul_cbs);
}

ARRAY_LIST_IMPL(array_list_l, long);
array_list_l_cbs al_l_cbs = { long_eq, NULL, NULL };

inline array_list_l*
al_l_create(void)
{
    return array_list_l_create(al_l_cbs);
}

ARRAY_LIST_IMPL(array_list_node, node_t*);
static array_list_node_cbs list_node_cbs = { node_equals, NULL, NULL };

inline array_list_node*
al_node_create(void)
{
    return array_list_node_create(list_node_cbs);
}

ARRAY_LIST_IMPL(array_list_relationship, relationship_t*);
array_list_relationship_cbs list_rel_cbs = { relationship_equals, NULL, NULL };

inline array_list_relationship*
al_rel_create(void)
{
    return array_list_relationship_create(list_rel_cbs);
}

