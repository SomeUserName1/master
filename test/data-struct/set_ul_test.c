#include "../../src/data-struct/set.h"
#include "../../src/data-struct/set_ul.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/data-struct/htable.h"

#define MAGIC_TEST_VALUE (42777)
#define ANOTHER_TV       (11)
#define NON_EX_VAL       (666)

void
test_create_set_ul(void)
{
    set_ul_t* set = create_set_ul();

    assert(set);
    assert(((htable_t*)set)->num_used == 0);
    assert(((htable_t*)set)->num_buckets == BUCKET_START);

    set_ul_destroy(set);
}

void
test_set_ul_size(void)
{
    set_ul_t* set = create_set_ul();

    assert(((htable_t*)set)->num_used == set_ul_size(set));

    unsigned long value = MAGIC_TEST_VALUE;
    set_insert((set_t*)set, (void*)&value);

    assert(((htable_t*)set)->num_used == set_ul_size(set));
    assert(1 == set_ul_size(set));

    set_ul_destroy(set);
}

void
test_set_ul_insert(void)
{
    set_ul_t* set = create_set_ul();
    assert(0 == set_ul_size(set));

    if (set_ul_insert(set, MAGIC_TEST_VALUE)) {
        printf("Insertion failed");
        exit(-1);
    }
    assert(1 == set_ul_size(set));

    set_ul_insert(set, 0);
    assert(2 == set_ul_size(set));

    set_ul_insert(set, ANOTHER_TV);
    assert(3 == set_ul_size(set));

    set_ul_insert(set, 1);
    assert(4 == set_ul_size(set));

    set_ul_destroy(set);
}

void
test_set_ul_remove(void)
{
    set_ul_t* set = create_set_ul();

    set_ul_insert(set, MAGIC_TEST_VALUE);
    set_ul_insert(set, 0);
    assert(2 == set_ul_size(set));
    assert(set_ul_contains(set, MAGIC_TEST_VALUE));
    assert(set_ul_contains(set, 0));

    set_ul_remove(set, 0);
    assert(1 == set_ul_size(set));
    set_ul_remove(set, MAGIC_TEST_VALUE);
    assert(0 == set_ul_size(set));

    set_ul_destroy(set);
}

void
test_set_ul_contains(void)
{
    set_ul_t* set = create_set_ul();

    assert(0 == set_ul_size(set));

    set_ul_insert(set, MAGIC_TEST_VALUE);
    set_ul_insert(set, 0);

    assert(set_ul_contains(set, MAGIC_TEST_VALUE));
    assert(set_ul_contains(set, 0));
    assert(!set_ul_contains(set, NON_EX_VAL));

    set_ul_destroy(set);
}

void
test_set_ul_destroy(void)
{
    set_ul_t* lst = create_set_ul();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        set_ul_insert(lst, i);
    }
    set_ul_destroy(lst);
}

int
main(void)
{
    test_create_set_ul();
    test_set_ul_size();
    test_set_ul_insert();
    test_set_ul_remove();
    test_set_ul_contains();
    test_set_ul_destroy();
}
