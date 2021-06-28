#include "data-struct/linked_list.h"
#include "data-struct/cbs.h"

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

