/*
 * @(#)linked_list.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "data-struct/linked_list.h"

#include "data-struct/cbs.h"

LINKED_LIST_IMPL(llist_ul, unsigned long);
llist_ul_cbs ll_ul_cbs = { unsigned_long_eq, NULL, NULL };

llist_ul*
ll_ul_create(void)
{
    return llist_ul_create(ll_ul_cbs);
}

QUEUE_IMPL(queue_ul, unsigned long);
queue_ul_cbs q_ul_cbs = { unsigned_long_eq, NULL, NULL };

queue_ul*
q_ul_create(void)
{
    return queue_ul_create(q_ul_cbs);
}

STACK_IMPL(stack_ul, unsigned long);
stack_ul_cbs st_ul_cbs = { unsigned_long_eq, NULL, NULL };

stack_ul*
st_ul_create(void)
{
    return stack_ul_create(st_ul_cbs);
}

