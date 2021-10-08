/*
 * set_ul_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "data-struct/set.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MAGIC_TEST_VALUE (42777)
#define ANOTHER_TV       (11)
#define NON_EX_VAL       (666)

void
test_s_ul_create(void)
{
    set_ul* set = s_ul_create();

    assert(set);
    assert(set->num_used == 0);
    assert(set->num_buckets == BUCKET_START_S);

    set_ul_destroy(set);
}

void
test_set_ul_size(void)
{
    set_ul* set = s_ul_create();

    assert(set->num_used == set_ul_size(set));

    unsigned long value = MAGIC_TEST_VALUE;
    set_ul_insert(set, value);

    assert(set->num_used == set_ul_size(set));
    assert(1 == set_ul_size(set));

    set_ul_destroy(set);
}

void
test_set_ul_insert(void)
{
    set_ul* set = s_ul_create();
    assert(0 == set_ul_size(set));

    if (set_ul_insert(set, MAGIC_TEST_VALUE)) {
        printf("Insertion failed");
        exit(EXIT_FAILURE);
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
    set_ul* set = s_ul_create();

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
    set_ul* set = s_ul_create();

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
    set_ul* lst = s_ul_create();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        set_ul_insert(lst, i);
    }
    set_ul_destroy(lst);
}

int
main(void)
{
    test_s_ul_create();
    test_set_ul_size();
    test_set_ul_insert();
    test_set_ul_remove();
    test_set_ul_contains();
    test_set_ul_destroy();
}
