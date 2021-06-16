#include "data-struct/queue.h"
#include "data-struct/queue_ul.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define MAGIC_TEST_VALUE (42777)
#define ANOTHER_TV       (11)
#define NON_EX_VAL       (666)

void
test_create_queue_ul(void)
{
    queue_ul_t* queue = create_queue_ul();

    assert(queue);
    assert(((queue_t*)queue)->len == 0);

    queue_ul_destroy(queue);
}

void
test_queue_ul_size(void)
{
    queue_ul_t* queue = create_queue_ul();

    assert(((queue_t*)queue)->len == queue_ul_size(queue));

    unsigned long value = MAGIC_TEST_VALUE;
    queue_add((queue_t*)queue, (void*)&value);

    assert(((queue_t*)queue)->len == queue_ul_size(queue));
    assert(1 == queue_ul_size(queue));

    queue_ul_destroy(queue);
}

void
test_queue_ul_add(void)
{
    queue_ul_t* queue = create_queue_ul();

    assert(0 == queue_ul_size(queue));

    queue_ul_add(queue, MAGIC_TEST_VALUE);
    assert(((queue_t*)queue)->head == ((queue_t*)queue)->tail);
    assert(1 == queue_ul_size(queue));

    queue_ul_add(queue, 0);
    assert(2 == queue_ul_size(queue));
    assert(((queue_t*)queue)->head != ((queue_t*)queue)->tail);
    assert(((queue_t*)queue)->head->next == ((queue_t*)queue)->tail);
    assert(*((unsigned long*)((queue_t*)queue)->tail->element) == 0);

    queue_ul_destroy(queue);
}

void
test_queue_ul_insert(void)
{
    queue_ul_t* queue = create_queue_ul();
    assert(0 == queue_ul_size(queue));

    if (queue_ul_insert(queue, MAGIC_TEST_VALUE, 0)) {
        printf("Insertion failed");
        exit(-1);
    }
    assert(((queue_t*)queue)->head == ((queue_t*)queue)->tail);
    assert(*((unsigned long*)((queue_t*)queue)->head->element)
           == MAGIC_TEST_VALUE);
    assert(1 == queue_ul_size(queue));

    queue_ul_insert(queue, 0, 0);
    assert(2 == queue_ul_size(queue));
    assert(*((unsigned long*)((queue_t*)queue)->head->element) == 0);

    queue_ul_insert(queue, ANOTHER_TV, 1);
    assert(3 == queue_ul_size(queue));
    assert(*((unsigned long*)((queue_t*)queue)->head->element) == 0);

    assert(*((unsigned long*)((queue_t*)queue)->head->next->element)
           == ANOTHER_TV);
    assert(*((unsigned long*)((queue_t*)queue)->head->next->next->element)
           == MAGIC_TEST_VALUE);
    assert(*((unsigned long*)((queue_t*)queue)->head->next->next->next->element)
           == 0);

    queue_ul_insert(queue, 1, 1);
    assert(4 == queue_ul_size(queue));
    assert(*((unsigned long*)((queue_t*)queue)->head->element) == 0);
    assert(*((unsigned long*)((queue_t*)queue)->head->next->element) == 1);
    assert(*((unsigned long*)((queue_t*)queue)->head->next->next->element)
           == ANOTHER_TV);
    assert(*((unsigned long*)((queue_t*)queue)->head->next->next->next->element)
           == MAGIC_TEST_VALUE);

    queue_ul_destroy(queue);
}

void
test_queue_ul_remove(void)
{
    queue_ul_t* queue = create_queue_ul();

    queue_ul_add(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);
    assert(2 == queue_ul_size(queue));

    queue_ul_remove(queue, 0);
    assert(1 == queue_ul_size(queue));
    assert(*((unsigned long*)((queue_t*)queue)->head->element)
           == MAGIC_TEST_VALUE);

    queue_ul_remove(queue, 0);
    assert(0 == queue_ul_size(queue));

    queue_ul_destroy(queue);
}

void
test_queue_ul_remove_elem(void)
{
    queue_ul_t* queue = create_queue_ul();

    assert(0 == queue_ul_size(queue));

    queue_ul_add(queue, MAGIC_TEST_VALUE);

    assert(1 == queue_ul_size(queue));

    queue_ul_insert(queue, 0, 0);
    assert(2 == queue_ul_size(queue));

    queue_ul_remove_elem(queue, 0);
    assert(1 == queue_ul_size(queue));
    assert(*((unsigned long*)((queue_t*)queue)->head->element)
           == MAGIC_TEST_VALUE);

    queue_ul_remove_elem(queue, MAGIC_TEST_VALUE);
    assert(0 == queue_ul_size(queue));

    queue_ul_destroy(queue);
}

void
test_queue_ul_index_of(void)
{
    queue_ul_t* queue = create_queue_ul();
    queue_ul_add(queue, MAGIC_TEST_VALUE);
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
    queue_ul_t* queue = create_queue_ul();

    assert(0 == queue_ul_size(queue));

    queue_ul_add(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);

    assert(queue_ul_contains(queue, MAGIC_TEST_VALUE));
    assert(!queue_ul_contains(queue, NON_EX_VAL));

    queue_ul_destroy(queue);
}

void
test_queue_ul_get(void)
{
    queue_ul_t* queue = create_queue_ul();
    queue_ul_add(queue, MAGIC_TEST_VALUE);
    queue_ul_insert(queue, 0, 0);

    assert(0 == queue_ul_get(queue, 0));
    assert(MAGIC_TEST_VALUE == queue_ul_get(queue, 1));

    queue_ul_destroy(queue);
}

void
test_queue_ul_take(void)
{
    queue_ul_t* queue = create_queue_ul();
    queue_ul_add(queue, MAGIC_TEST_VALUE);

    unsigned long* num = queue_ul_take(queue);
    assert(*num == MAGIC_TEST_VALUE);
    free(num);
    queue_ul_destroy(queue);
}

void
test_queue_ul_destroy(void)
{
    queue_ul_t* lst = create_queue_ul();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        queue_ul_add(lst, i);
    }
    queue_ul_destroy(lst);
}

int
main(void)
{
    test_create_queue_ul();
    test_queue_ul_size();
    test_queue_ul_add();
    test_queue_ul_insert();
    test_queue_ul_remove();
    test_queue_ul_remove_elem();
    test_queue_ul_index_of();
    test_queue_ul_contains();
    test_queue_ul_get();
    test_queue_ul_take();
    test_queue_ul_destroy();
}
