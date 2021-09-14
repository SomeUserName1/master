/*
 * @(#)queue_ul_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "data-struct/linked_list.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define MAGIC_TEST_VALUE (42777)
#define ANOTHER_TV       (11)
#define NON_EX_VAL       (666)

void
test_q_ul_create(void)
{
    queue_ul* queue = q_ul_create();

    assert(queue);
    assert(queue->len == 0);

    queue_ul_destroy(queue);
}

void
test_queue_ul_size(void)
{
    queue_ul* queue = q_ul_create();

    assert(queue->len == queue_ul_size(queue));

    unsigned long value = MAGIC_TEST_VALUE;
    queue_ul_push(queue, value);

    assert(queue->len == queue_ul_size(queue));
    assert(1 == queue_ul_size(queue));

    queue_ul_destroy(queue);
}

void
test_queue_ul_push(void)
{
    queue_ul* queue = q_ul_create();

    assert(0 == queue_ul_size(queue));

    queue_ul_push(queue, MAGIC_TEST_VALUE);
    assert(queue->head == queue->tail);
    assert(1 == queue_ul_size(queue));

    queue_ul_push(queue, 0);
    assert(2 == queue_ul_size(queue));
    assert(queue->head != queue->tail);
    assert(queue->head->next == queue->tail);
    assert(queue->tail->element == 0);

    queue_ul_destroy(queue);
}

void
test_queue_ul_insert(void)
{
    queue_ul* queue = q_ul_create();
    assert(0 == queue_ul_size(queue));

    queue_ul_insert(queue, MAGIC_TEST_VALUE, 0);
    assert(queue->head == queue->tail);
    assert(1 == queue_ul_size(queue));
    assert(queue->head->element == MAGIC_TEST_VALUE);

    queue_ul_insert(queue, 0, 0);
    assert(2 == queue_ul_size(queue));
    assert(queue->head->element == 0);

    queue_ul_insert(queue, ANOTHER_TV, 1);
    assert(3 == queue_ul_size(queue));
    assert(queue->head->element == 0);

    assert(queue->head->next->element == ANOTHER_TV);
    assert(queue->head->next->next->element == MAGIC_TEST_VALUE);
    assert(queue->head->next->next->next->element == 0);

    queue_ul_insert(queue, 1, 1);
    assert(4 == queue_ul_size(queue));
    assert(queue->head->element == 0);
    assert(queue->head->next->element == 1);
    assert(queue->head->next->next->element == ANOTHER_TV);
    assert(queue->head->next->next->next->element == MAGIC_TEST_VALUE);

    queue_ul_destroy(queue);
}

void
test_queue_ul_remove(void)
{
    queue_ul* queue = q_ul_create();

    queue_ul_push(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);
    assert(2 == queue_ul_size(queue));

    queue_ul_remove(queue, 0);
    assert(1 == queue_ul_size(queue));
    assert(queue->head->element == MAGIC_TEST_VALUE);

    queue_ul_remove(queue, 0);
    assert(0 == queue_ul_size(queue));

    queue_ul_destroy(queue);
}

void
test_queue_ul_remove_elem(void)
{
    queue_ul* queue = q_ul_create();

    assert(0 == queue_ul_size(queue));

    queue_ul_push(queue, MAGIC_TEST_VALUE);

    assert(1 == queue_ul_size(queue));

    queue_ul_insert(queue, 0, 0);
    assert(2 == queue_ul_size(queue));

    queue_ul_remove_elem(queue, 0);
    assert(1 == queue_ul_size(queue));
    assert(queue->head->element == MAGIC_TEST_VALUE);

    queue_ul_remove_elem(queue, MAGIC_TEST_VALUE);
    assert(0 == queue_ul_size(queue));

    queue_ul_destroy(queue);
}

void
test_queue_ul_index_of(void)
{
    queue_ul* queue = q_ul_create();
    queue_ul_push(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == queue_ul_index_of(queue, MAGIC_TEST_VALUE, &index));
    assert(1 == index);

    assert(-1 == queue_ul_index_of(queue, NON_EX_VAL, &index));

    queue_ul_destroy(queue);
}

void
test_queue_ul_contains(void)
{
    queue_ul* queue = q_ul_create();

    assert(0 == queue_ul_size(queue));

    queue_ul_push(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);

    assert(queue_ul_contains(queue, MAGIC_TEST_VALUE));
    assert(!queue_ul_contains(queue, NON_EX_VAL));

    queue_ul_destroy(queue);
}

void
test_queue_ul_get(void)
{
    queue_ul* queue = q_ul_create();
    queue_ul_push(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);

    assert(0 == queue_ul_get(queue, 0));
    assert(MAGIC_TEST_VALUE == queue_ul_get(queue, 1));

    queue_ul_destroy(queue);
}

void
test_queue_ul_pop(void)
{
    queue_ul* queue = q_ul_create();
    queue_ul_push(queue, MAGIC_TEST_VALUE);

    unsigned long num = queue_ul_pop(queue);
    assert(num == MAGIC_TEST_VALUE);
    queue_ul_destroy(queue);
}

void
test_queue_ul_destroy(void)
{
    queue_ul* lst = q_ul_create();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        queue_ul_push(lst, i);
    }
    queue_ul_destroy(lst);
}

void
test_queue_ul_move_back(void)
{
    queue_ul* lst = q_ul_create();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        queue_ul_push(lst, i);
    }
    assert(queue_ul_size(lst) == MAGIC_TEST_VALUE);

    for (size_t i = 0; i < ANOTHER_TV; ++i) {
        queue_ul_move_back(lst, rand() % (MAGIC_TEST_VALUE));
    }
    assert(queue_ul_size(lst) == MAGIC_TEST_VALUE);

    queue_ul_destroy(lst);
}

int
main(void)
{
    test_q_ul_create();
    test_queue_ul_size();
    test_queue_ul_push();
    test_queue_ul_insert();
    test_queue_ul_remove();
    test_queue_ul_remove_elem();
    test_queue_ul_index_of();
    test_queue_ul_contains();
    test_queue_ul_get();
    test_queue_ul_pop();
    test_queue_ul_destroy();
    test_queue_ul_move_back();
}
