#include "page.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"

HTABLE_IMPL(dict_ul_page, unsigned long, page*, fnv_hash_ul, unsigned_long_eq);
dict_ul_page_cbs d_page_cbs = { NULL, NULL, unsigned_long_print, page_equals,
                                NULL, NULL, page_pretty_print };

dict_ul_page*
d_ul_page_create(void)
{
    return dict_ul_page_create(d_page_cbs);
}

LINKED_LIST_IMPL(linked_list_page, page*);
linked_list_page_cbs ll_page_cbs = { page_equals, NULL, NULL };

linked_list_page*
ll_page_create(void)
{
    return linked_list_page_create(ll_page_cbs);
}

