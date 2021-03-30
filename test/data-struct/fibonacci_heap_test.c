#include "../../src/data-struct/fibonacci_heap.h"

#include <assert.h>
#include <stdio.h>

void
test_create_fh(void)
{
    fib_heap_t* fh = create_fib_heap();
    assert(fh->num_nodes == 0);
}

void
test_create_fh_node(void)
{}

void
test_fh_insert(void)
{}

void
test_fh_min(void)
{}

void
test_fh_extract_min(void)
{}

void
test_union(void)
{}

void
testfh_decrease_key(void)
{}

void
test_fh_delete(void)
{}

void
test_fh_destroy(void)
{}

int
main(void)
{
    printf("Not implemented.");
}
