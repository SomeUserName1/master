#include "../../src/data-struct/list.h"
#include "../../src/data-struct/list_l.h"
#include "../../src/data-struct/list_node.h"
#include "../../src/data-struct/list_rel.h"
#include "../../src/data-struct/list_ul.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "../../src/record/node.h"
#include "../../src/record/relationship.h"

#define MAGIC_TEST_VALUE (42777)
#define NON_EX_VAL       (666)

void
test_create_list_ul(void)
{
    list_ul_t* list = create_list_ul();

    assert(list);
    assert(((list_t*)list)->len == 0);
    assert(((list_t*)list)->alloced == 128);

    list_ul_destroy(list);
}

void
test_list_ul_size(void)
{
    list_ul_t* list = create_list_ul();

    assert(((list_t*)list)->len == list_ul_size(list));

    unsigned long value = MAGIC_TEST_VALUE;
    list_append((list_t*)list, (void*)&value);

    assert(((list_t*)list)->len == list_ul_size(list));
    assert(1 == list_ul_size(list));

    list_ul_destroy(list);
}

void
test_list_ul_append(void)
{
    list_ul_t* list = create_list_ul();

    assert(0 == list_ul_size(list));

    list_ul_append(list, MAGIC_TEST_VALUE);

    assert(1 == list_ul_size(list));

    list_ul_append(list, 0);
    assert(2 == list_ul_size(list));
    assert(*((unsigned long*)((list_t*)list)->elements[1]) == 0);

    list_ul_destroy(list);
}

void
test_list_ul_insert(void)
{
    list_ul_t* list = create_list_ul();

    assert(0 == list_ul_size(list));

    list_ul_append(list, MAGIC_TEST_VALUE);

    assert(1 == list_ul_size(list));

    list_ul_insert(list, 0, 0);
    assert(2 == list_ul_size(list));
    assert(*((unsigned long*)((list_t*)list)->elements[0]) == 0);

    list_ul_destroy(list);
}

void
test_list_ul_remove(void)
{
    list_ul_t* list = create_list_ul();

    list_ul_append(list, MAGIC_TEST_VALUE);
    list_ul_insert(list, 0, 0);
    assert(2 == list_ul_size(list));

    list_ul_remove(list, 0);
    assert(1 == list_ul_size(list));
    assert(*((unsigned long*)((list_t*)list)->elements[0]) == MAGIC_TEST_VALUE);

    list_ul_remove(list, 0);
    assert(0 == list_ul_size(list));

    list_ul_destroy(list);
}

void
test_list_ul_remove_elem(void)
{
    list_ul_t* list = create_list_ul();

    assert(0 == list_ul_size(list));

    list_ul_append(list, MAGIC_TEST_VALUE);

    assert(1 == list_ul_size(list));

    list_ul_insert(list, 0, 0);
    assert(2 == list_ul_size(list));

    list_ul_remove_elem(list, 0);
    assert(1 == list_ul_size(list));
    assert(*((unsigned long*)((list_t*)list)->elements[0]) == MAGIC_TEST_VALUE);

    list_ul_remove_elem(list, MAGIC_TEST_VALUE);
    assert(0 == list_ul_size(list));

    list_ul_destroy(list);
}

void
test_list_ul_index_of(void)
{
    list_ul_t* list = create_list_ul();

    list_ul_append(list, MAGIC_TEST_VALUE);
    list_ul_insert(list, 0, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == list_ul_index_of(list, MAGIC_TEST_VALUE, &index));
    assert(1 == index);

    assert(-1 == list_ul_index_of(list, NON_EX_VAL, &index));

    list_ul_destroy(list);
}

void
test_list_ul_contains(void)
{
    list_ul_t* list = create_list_ul();

    assert(0 == list_ul_size(list));

    list_ul_append(list, MAGIC_TEST_VALUE);
    list_ul_insert(list, 0, 0);

    assert(list_ul_contains(list, MAGIC_TEST_VALUE));
    assert(!list_ul_contains(list, NON_EX_VAL));

    list_ul_destroy(list);
}

void
test_list_ul_get(void)
{
    list_ul_t* list = create_list_ul();
    list_ul_append(list, MAGIC_TEST_VALUE);
    list_ul_insert(list, 0, 0);

    assert(0 == list_ul_get(list, 0));
    assert(MAGIC_TEST_VALUE == list_ul_get(list, 1));

    list_ul_destroy(list);
}

void
test_list_ul_take(void)
{
    list_ul_t* list = create_list_ul();
    list_ul_append(list, MAGIC_TEST_VALUE);

    unsigned long* num = list_ul_take(list, 0);
    assert(*num == MAGIC_TEST_VALUE);
    free(num);
    list_ul_destroy(list);
}

// Used to check for memory leaks
void
test_list_ul_destroy(void)
{
    list_ul_t* lst = create_list_ul();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        list_ul_append(lst, i);
    }
    list_ul_destroy(lst);
}

void
test_create_list_l(void)
{
    list_l_t* list = create_list_l();

    assert(list);
    assert(((list_t*)list)->len == 0);
    assert(((list_t*)list)->alloced == 128);

    list_l_destroy(list);
}

void
test_list_l_size(void)
{
    list_l_t* list = create_list_l();

    assert(((list_t*)list)->len == list_l_size(list));

    long value = MAGIC_TEST_VALUE;
    list_append((list_t*)list, (void*)&value);

    assert(((list_t*)list)->len == list_l_size(list));
    assert(1 == list_l_size(list));

    list_l_destroy(list);
}

void
test_list_l_append(void)
{
    list_l_t* list = create_list_l();

    assert(0 == list_l_size(list));

    list_l_append(list, MAGIC_TEST_VALUE);

    assert(1 == list_l_size(list));

    list_l_append(list, 0);
    assert(2 == list_l_size(list));
    assert(*((long*)((list_t*)list)->elements[1]) == 0);

    list_l_destroy(list);
}

void
test_list_l_insert(void)
{
    list_l_t* list = create_list_l();

    assert(0 == list_l_size(list));

    list_l_append(list, MAGIC_TEST_VALUE);

    assert(1 == list_l_size(list));

    list_l_insert(list, 0, 0);
    assert(2 == list_l_size(list));
    assert(*((long*)((list_t*)list)->elements[0]) == 0);

    list_l_destroy(list);
}

void
test_list_l_remove(void)
{
    list_l_t* list = create_list_l();

    list_l_append(list, MAGIC_TEST_VALUE);
    list_l_insert(list, 0, 0);
    assert(2 == list_l_size(list));

    list_l_remove(list, 0);
    assert(1 == list_l_size(list));
    assert(*((long*)((list_t*)list)->elements[0]) == MAGIC_TEST_VALUE);

    list_l_remove(list, 0);
    assert(0 == list_l_size(list));

    list_l_destroy(list);
}

void
test_list_l_remove_elem(void)
{
    list_l_t* list = create_list_l();

    assert(0 == list_l_size(list));

    list_l_append(list, MAGIC_TEST_VALUE);

    assert(1 == list_l_size(list));

    list_l_insert(list, 0, 0);
    assert(2 == list_l_size(list));

    list_l_remove_elem(list, 0);
    assert(1 == list_l_size(list));
    assert(*((long*)((list_t*)list)->elements[0]) == MAGIC_TEST_VALUE);

    list_l_remove_elem(list, MAGIC_TEST_VALUE);
    assert(0 == list_l_size(list));

    list_l_destroy(list);
}

void
test_list_l_index_of(void)
{
    list_l_t* list = create_list_l();

    list_l_append(list, MAGIC_TEST_VALUE);
    list_l_insert(list, 0, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == list_l_index_of(list, MAGIC_TEST_VALUE, &index));
    assert(1 == index);

    assert(-1 == list_l_index_of(list, NON_EX_VAL, &index));

    list_l_destroy(list);
}

void
test_list_l_contains(void)
{
    list_l_t* list = create_list_l();

    assert(0 == list_l_size(list));

    list_l_append(list, MAGIC_TEST_VALUE);
    list_l_insert(list, 0, 0);

    assert(list_l_contains(list, MAGIC_TEST_VALUE));
    assert(!list_l_contains(list, NON_EX_VAL));

    list_l_destroy(list);
}

void
test_list_l_get(void)
{
    list_l_t* list = create_list_l();
    list_l_append(list, MAGIC_TEST_VALUE);
    list_l_insert(list, 0, 0);

    assert(0 == list_l_get(list, 0));
    assert(MAGIC_TEST_VALUE == list_l_get(list, 1));

    list_l_destroy(list);
}

void
test_list_l_take(void)
{
    list_l_t* list = create_list_l();
    list_l_append(list, MAGIC_TEST_VALUE);

    long* num = list_l_take(list, 0);
    assert(*num == MAGIC_TEST_VALUE);
    free(num);
    list_l_destroy(list);
}

// Used to check for memory leaks
void
test_list_l_destroy(void)
{
    list_l_t* lst = create_list_l();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        list_l_append(lst, i);
    }
    list_l_destroy(lst);
}

void
test_create_list_node(void)
{
    list_node_t* list = create_list_node();

    assert(list);
    assert(((list_t*)list)->len == 0);
    assert(((list_t*)list)->alloced == 128);

    list_node_destroy(list);
}

void
test_list_node_size(void)
{
    list_node_t* list = create_list_node();

    assert(((list_t*)list)->len == list_node_size(list));

    node_t* value = new_node();
    list_append((list_t*)list, (void*)value);

    assert(((list_t*)list)->len == list_node_size(list));
    assert(1 == list_node_size(list));

    free(value);
    list_node_destroy(list);
}

void
test_list_node_append(void)
{
    list_node_t* list = create_list_node();
    assert(0 == list_node_size(list));

    node_t* val = new_node();
    val->id     = 0;
    list_node_append(list, val);
    assert(1 == list_node_size(list));

    node_t* val1 = new_node();
    val1->id     = 1;
    list_node_append(list, val1);
    assert(2 == list_node_size(list));

    assert(node_equals(((node_t*)((list_t*)list)->elements[1]), val1));

    free(val);
    free(val1);
    list_node_destroy(list);
}

void
test_list_node_insert(void)
{
    list_node_t* list = create_list_node();

    assert(0 == list_node_size(list));

    node_t* val = new_node();
    val->id     = 0;
    list_node_append(list, val);

    assert(1 == list_node_size(list));

    node_t* val1 = new_node();
    val1->id     = 1;
    list_node_insert(list, val1, 0);
    assert(2 == list_node_size(list));
    assert(node_equals(((node_t*)((list_t*)list)->elements[0]), val1));

    free(val);
    free(val1);
    list_node_destroy(list);
}

void
test_list_node_remove(void)
{
    list_node_t* list = create_list_node();

    node_t* val  = new_node();
    node_t* val1 = new_node();
    val->id      = 0;
    val1->id     = 1;

    list_node_append(list, val);
    list_node_insert(list, val1, 0);
    assert(2 == list_node_size(list));

    list_node_remove(list, 0);
    assert(1 == list_node_size(list));

    assert(node_equals(((node_t*)((list_t*)list)->elements[0]), val));

    list_node_remove(list, 0);
    assert(0 == list_node_size(list));

    free(val);
    free(val1);
    list_node_destroy(list);
}

void
test_list_node_remove_elem(void)
{
    list_node_t* list = create_list_node();

    node_t* val  = new_node();
    node_t* val1 = new_node();
    val->id      = 0;
    val1->id     = 1;

    list_node_append(list, val);
    list_node_insert(list, val1, 0);
    assert(2 == list_node_size(list));

    list_node_remove_elem(list, val1);
    assert(1 == list_node_size(list));

    assert(node_equals(((node_t*)((list_t*)list)->elements[0]), val));

    list_node_remove_elem(list, val);
    assert(0 == list_node_size(list));

    free(val);
    free(val1);
    list_node_destroy(list);
}

void
test_list_node_index_of(void)
{

    list_node_t* list = create_list_node();

    node_t* val     = new_node();
    node_t* val1    = new_node();
    node_t* another = new_node();
    val->id         = 0;
    val1->id        = 1;
    another->id     = 2;
    list_node_append(list, val);
    list_node_insert(list, val1, 0);

    for (size_t i = 0; i < list_node_size(list); ++i) {
        node_pretty_print(list_node_get(list, i));
    }

    unsigned long index = ULONG_MAX;
    assert(0 == list_node_index_of(list, val, &index));
    assert(1 == index);

    assert(-1 == list_node_index_of(list, another, &index));

    free(val);
    free(val1);
    free(another);
    list_node_destroy(list);
}

void
test_list_node_contains(void)
{
    list_node_t* list = create_list_node();

    node_t* val     = new_node();
    node_t* val1    = new_node();
    node_t* another = new_node();
    val->id         = 0;
    val1->id        = 1;
    another->id     = 2;
    list_node_append(list, val);
    list_node_insert(list, val1, 0);

    assert(list_node_contains(list, val));
    assert(!list_node_contains(list, another));

    free(val);
    free(val1);
    free(another);
    list_node_destroy(list);
}

void
test_list_node_get(void)
{
    list_node_t* list = create_list_node();

    node_t* val  = new_node();
    node_t* val1 = new_node();
    val->id      = 0;
    val1->id     = 1;
    list_node_append(list, val);
    list_node_insert(list, val1, 0);

    assert(node_equals(val1, list_node_get(list, 0)));
    assert(node_equals(val, list_node_get(list, 1)));

    free(val);
    free(val1);
    list_node_destroy(list);
}

void
test_list_node_take(void)
{
    list_node_t* list = create_list_node();
    node_t*      val  = new_node();
    val->id           = 0;
    list_node_append(list, val);

    node_t* num = list_node_take(list, 0);
    assert(node_equals(num, val));
    free(num);
    list_node_destroy(list);
}

// Used to check for memory leaks
void
test_list_node_destroy(void)
{
    list_node_t* lst = create_list_node();
    node_t*      n;
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        n     = new_node();
        n->id = i;
        list_node_append(lst, n);
        free(n);
    }

    list_node_destroy(lst);
}

void
test_create_list_relationship(void)
{
    list_relationship_t* list = create_list_relationship();

    assert(list);
    assert(((list_t*)list)->len == 0);
    assert(((list_t*)list)->alloced == 128);

    list_relationship_destroy(list);
}

void
test_list_relationship_size(void)
{
    list_relationship_t* list = create_list_relationship();

    assert(((list_t*)list)->len == list_relationship_size(list));

    relationship_t* value = new_relationship();
    list_append((list_t*)list, (void*)value);

    assert(((list_t*)list)->len == list_relationship_size(list));
    assert(1 == list_relationship_size(list));

    free(value);
    list_relationship_destroy(list);
}

void
test_list_relationship_append(void)
{
    list_relationship_t* list = create_list_relationship();

    assert(0 == list_relationship_size(list));

    relationship_t* val = new_relationship();
    val->id             = 0;
    list_relationship_append(list, val);

    assert(1 == list_relationship_size(list));

    relationship_t* val1 = new_relationship();
    val1->id             = 1;
    list_relationship_append(list, val1);

    for (size_t i = 0; i < list_relationship_size(list); ++i) {
        relationship_pretty_print(list_relationship_get(list, i));
    }

    assert(2 == list_relationship_size(list));
    assert(relationship_equals(((relationship_t*)((list_t*)list)->elements[1]),
                               val1));

    free(val);
    free(val1);
    list_relationship_destroy(list);
}

void
test_list_relationship_insert(void)
{
    list_relationship_t* list = create_list_relationship();

    assert(0 == list_relationship_size(list));

    relationship_t* val = new_relationship();
    val->id             = 0;
    list_relationship_append(list, val);

    assert(1 == list_relationship_size(list));

    relationship_t* val1 = new_relationship();
    val1->id             = 1;
    list_relationship_insert(list, val1, 0);
    assert(2 == list_relationship_size(list));
    assert(relationship_equals(((relationship_t*)((list_t*)list)->elements[0]),
                               val1));

    free(val);
    free(val1);
    list_relationship_destroy(list);
}

void
test_list_relationship_remove(void)
{
    list_relationship_t* list = create_list_relationship();

    relationship_t* val  = new_relationship();
    relationship_t* val1 = new_relationship();
    val->id              = 0;
    val1->id             = 1;

    list_relationship_append(list, val);
    list_relationship_insert(list, val1, 0);
    assert(2 == list_relationship_size(list));

    list_relationship_remove(list, 0);
    assert(1 == list_relationship_size(list));

    assert(relationship_equals(((relationship_t*)((list_t*)list)->elements[0]),
                               val));

    list_relationship_remove(list, 0);
    assert(0 == list_relationship_size(list));

    free(val);
    free(val1);
    list_relationship_destroy(list);
}

void
test_list_relationship_remove_elem(void)
{
    list_relationship_t* list = create_list_relationship();

    relationship_t* val  = new_relationship();
    relationship_t* val1 = new_relationship();
    val->id              = 0;
    val1->id             = 1;

    list_relationship_append(list, val);
    list_relationship_insert(list, val1, 0);
    assert(2 == list_relationship_size(list));

    list_relationship_remove_elem(list, val1);
    assert(1 == list_relationship_size(list));

    assert(relationship_equals(((relationship_t*)((list_t*)list)->elements[0]),
                               val));

    list_relationship_remove_elem(list, val);
    assert(0 == list_relationship_size(list));

    free(val);
    free(val1);
    list_relationship_destroy(list);
}

void
test_list_relationship_index_of(void)
{

    list_relationship_t* list = create_list_relationship();

    relationship_t* val     = new_relationship();
    relationship_t* val1    = new_relationship();
    relationship_t* another = new_relationship();
    val->id                 = 0;
    val1->id                = 1;
    another->id             = 2;
    list_relationship_append(list, val);
    list_relationship_insert(list, val1, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == list_relationship_index_of(list, val, &index));
    assert(1 == index);

    assert(-1 == list_relationship_index_of(list, another, &index));

    free(val);
    free(val1);
    free(another);
    list_relationship_destroy(list);
}

void
test_list_relationship_contains(void)
{
    list_relationship_t* list = create_list_relationship();

    relationship_t* val     = new_relationship();
    relationship_t* val1    = new_relationship();
    relationship_t* another = new_relationship();
    val->id                 = 0;
    val1->id                = 1;
    another->id             = 2;
    list_relationship_append(list, val);
    list_relationship_insert(list, val1, 0);

    assert(list_relationship_contains(list, val));
    assert(!list_relationship_contains(list, another));

    free(val);
    free(val1);
    free(another);
    list_relationship_destroy(list);
}

void
test_list_relationship_get(void)
{
    list_relationship_t* list = create_list_relationship();

    relationship_t* val  = new_relationship();
    relationship_t* val1 = new_relationship();
    val->id              = 0;
    val1->id             = 1;
    list_relationship_append(list, val);
    list_relationship_insert(list, val1, 0);

    assert(relationship_equals(val1, list_relationship_get(list, 0)));
    assert(relationship_equals(val, list_relationship_get(list, 1)));

    free(val);
    free(val1);
    list_relationship_destroy(list);
}

void
test_list_relationship_take(void)
{
    list_relationship_t* list = create_list_relationship();
    relationship_t*      val  = new_relationship();
    val->id                   = 0;
    list_relationship_append(list, val);

    relationship_t* num = list_relationship_take(list, 0);
    assert(relationship_equals(num, val));
    free(num);
    list_relationship_destroy(list);
}

// Used to check for memory leaks
void
test_list_relationship_destroy(void)
{
    list_relationship_t* lst = create_list_relationship();
    relationship_t*      n;
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        n     = new_relationship();
        n->id = i;
        list_relationship_append(lst, n);
        free(n);
    }

    list_relationship_destroy(lst);
}
int
main(void)
{
    test_create_list_ul();
    test_list_ul_size();
    test_list_ul_append();
    test_list_ul_insert();
    test_list_ul_remove();
    test_list_ul_remove_elem();
    test_list_ul_index_of();
    test_list_ul_contains();
    test_list_ul_get();
    test_list_ul_take();
    test_list_ul_destroy();

    test_create_list_l();
    test_list_l_size();
    test_list_l_append();
    test_list_l_insert();
    test_list_l_remove();
    test_list_l_remove_elem();
    test_list_l_index_of();
    test_list_l_contains();
    test_list_l_get();
    test_list_l_take();
    test_list_l_destroy();

    test_create_list_node();
    test_list_node_size();
    test_list_node_append();
    test_list_node_insert();
    test_list_node_remove();
    test_list_node_remove_elem();
    test_list_node_index_of();
    test_list_node_contains();
    test_list_node_get();
    test_list_node_take();
    test_list_node_destroy();

    test_create_list_relationship();
    test_list_relationship_size();
    test_list_relationship_append();
    test_list_relationship_insert();
    test_list_relationship_remove();
    test_list_relationship_remove_elem();
    test_list_relationship_index_of();
    test_list_relationship_contains();
    test_list_relationship_get();
    test_list_relationship_take();
    test_list_relationship_destroy();
}
