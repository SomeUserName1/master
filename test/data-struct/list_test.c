/*
 * list_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "data-struct/array_list.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"

#define MAGIC_TEST_VALUE (42777)
#define NON_EX_VAL       (666)

void
test_al_ul_create(void)
{
    array_list_ul* list = al_ul_create();

    assert(list);
    assert(list->len == 0);
    assert(list->alloced == 128);

    array_list_ul_destroy(list);
}

void
test_array_list_ul_size(void)
{
    array_list_ul* list = al_ul_create();

    assert(list->len == array_list_ul_size(list));

    unsigned long value = MAGIC_TEST_VALUE;
    array_list_ul_append(list, value);

    assert(list->len == array_list_ul_size(list));
    assert(1 == array_list_ul_size(list));

    array_list_ul_destroy(list);
}

void
test_array_list_ul_append(void)
{
    array_list_ul* list = al_ul_create();

    assert(0 == array_list_ul_size(list));

    array_list_ul_append(list, MAGIC_TEST_VALUE);

    assert(1 == array_list_ul_size(list));

    array_list_ul_append(list, 0);
    assert(2 == array_list_ul_size(list));
    assert(list->elements[1] == 0);

    array_list_ul_destroy(list);
}

void
test_array_list_ul_insert(void)
{
    array_list_ul* list = al_ul_create();

    assert(0 == array_list_ul_size(list));

    array_list_ul_append(list, MAGIC_TEST_VALUE);

    assert(1 == array_list_ul_size(list));

    array_list_ul_insert(list, 0, 0);
    assert(2 == array_list_ul_size(list));
    assert(list->elements[0] == 0);

    array_list_ul_destroy(list);
}

void
test_array_list_ul_remove(void)
{
    array_list_ul* list = al_ul_create();

    array_list_ul_append(list, MAGIC_TEST_VALUE);
    array_list_ul_insert(list, 0, 0);
    assert(2 == array_list_ul_size(list));

    array_list_ul_remove(list, 0);
    assert(1 == array_list_ul_size(list));
    assert(list->elements[0] == MAGIC_TEST_VALUE);

    array_list_ul_remove(list, 0);
    assert(0 == array_list_ul_size(list));

    array_list_ul_destroy(list);
}

void
test_array_list_ul_remove_elem(void)
{
    array_list_ul* list = al_ul_create();

    assert(0 == array_list_ul_size(list));

    array_list_ul_append(list, MAGIC_TEST_VALUE);

    assert(1 == array_list_ul_size(list));

    array_list_ul_insert(list, 0, 0);
    assert(2 == array_list_ul_size(list));

    array_list_ul_remove_elem(list, 0);
    assert(1 == array_list_ul_size(list));
    assert(list->elements[0] == MAGIC_TEST_VALUE);

    array_list_ul_remove_elem(list, MAGIC_TEST_VALUE);
    assert(0 == array_list_ul_size(list));

    array_list_ul_destroy(list);
}

void
test_array_list_ul_index_of(void)
{
    array_list_ul* list = al_ul_create();

    array_list_ul_append(list, MAGIC_TEST_VALUE);
    array_list_ul_insert(list, 0, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == array_list_ul_index_of(list, MAGIC_TEST_VALUE, &index));
    assert(1 == index);

    assert(-1 == array_list_ul_index_of(list, NON_EX_VAL, &index));

    array_list_ul_destroy(list);
}

void
test_array_list_ul_contains(void)
{
    array_list_ul* list = al_ul_create();

    assert(0 == array_list_ul_size(list));

    array_list_ul_append(list, MAGIC_TEST_VALUE);
    array_list_ul_insert(list, 0, 0);

    assert(array_list_ul_contains(list, MAGIC_TEST_VALUE));
    assert(!array_list_ul_contains(list, NON_EX_VAL));

    array_list_ul_destroy(list);
}

void
test_array_list_ul_get(void)
{
    array_list_ul* list = al_ul_create();
    array_list_ul_append(list, MAGIC_TEST_VALUE);
    array_list_ul_insert(list, 0, 0);

    assert(0 == array_list_ul_get(list, 0));
    assert(MAGIC_TEST_VALUE == array_list_ul_get(list, 1));

    array_list_ul_destroy(list);
}

void
test_array_list_ul_take(void)
{
    array_list_ul* list = al_ul_create();
    array_list_ul_append(list, MAGIC_TEST_VALUE);

    unsigned long num = array_list_ul_take(list, 0);
    assert(num == MAGIC_TEST_VALUE);
    array_list_ul_destroy(list);
}

// Used to check for memory leaks
void
test_array_list_ul_destroy(void)
{
    array_list_ul* lst = al_ul_create();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        array_list_ul_append(lst, i);
    }
    array_list_ul_destroy(lst);
}

void
test_al_l_create(void)
{
    array_list_l* list = al_l_create();

    assert(list);
    assert(list->len == 0);
    assert(list->alloced == 128);

    array_list_l_destroy(list);
}

void
test_array_list_l_size(void)
{
    array_list_l* list = al_l_create();

    assert(list->len == array_list_l_size(list));

    long value = MAGIC_TEST_VALUE;
    array_list_l_append(list, value);

    assert(list->len == array_list_l_size(list));
    assert(1 == array_list_l_size(list));

    array_list_l_destroy(list);
}

void
test_array_list_l_append(void)
{
    array_list_l* list = al_l_create();

    assert(0 == array_list_l_size(list));

    array_list_l_append(list, MAGIC_TEST_VALUE);

    assert(1 == array_list_l_size(list));

    array_list_l_append(list, 0);
    assert(2 == array_list_l_size(list));
    assert(list->elements[1] == 0);

    array_list_l_destroy(list);
}

void
test_array_list_l_insert(void)
{
    array_list_l* list = al_l_create();

    assert(0 == array_list_l_size(list));

    array_list_l_append(list, MAGIC_TEST_VALUE);

    assert(1 == array_list_l_size(list));

    array_list_l_insert(list, 0, 0);
    assert(2 == array_list_l_size(list));
    assert(list->elements[0] == 0);

    array_list_l_destroy(list);
}

void
test_array_list_l_remove(void)
{
    array_list_l* list = al_l_create();

    array_list_l_append(list, MAGIC_TEST_VALUE);
    array_list_l_insert(list, 0, 0);
    assert(2 == array_list_l_size(list));

    array_list_l_remove(list, 0);
    assert(1 == array_list_l_size(list));
    assert(list->elements[0] == MAGIC_TEST_VALUE);

    array_list_l_remove(list, 0);
    assert(0 == array_list_l_size(list));

    array_list_l_destroy(list);
}

void
test_array_list_l_remove_elem(void)
{
    array_list_l* list = al_l_create();

    assert(0 == array_list_l_size(list));

    array_list_l_append(list, MAGIC_TEST_VALUE);

    assert(1 == array_list_l_size(list));

    array_list_l_insert(list, 0, 0);
    assert(2 == array_list_l_size(list));

    array_list_l_remove_elem(list, 0);
    assert(1 == array_list_l_size(list));
    assert(list->elements[0] == MAGIC_TEST_VALUE);

    array_list_l_remove_elem(list, MAGIC_TEST_VALUE);
    assert(0 == array_list_l_size(list));

    array_list_l_destroy(list);
}

void
test_array_list_l_index_of(void)
{
    array_list_l* list = al_l_create();

    array_list_l_append(list, MAGIC_TEST_VALUE);
    array_list_l_insert(list, 0, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == array_list_l_index_of(list, MAGIC_TEST_VALUE, &index));
    assert(1 == index);

    assert(-1 == array_list_l_index_of(list, NON_EX_VAL, &index));

    array_list_l_destroy(list);
}

void
test_array_list_l_contains(void)
{
    array_list_l* list = al_l_create();

    assert(0 == array_list_l_size(list));

    array_list_l_append(list, MAGIC_TEST_VALUE);
    array_list_l_insert(list, 0, 0);

    assert(array_list_l_contains(list, MAGIC_TEST_VALUE));
    assert(!array_list_l_contains(list, NON_EX_VAL));

    array_list_l_destroy(list);
}

void
test_array_list_l_get(void)
{
    array_list_l* list = al_l_create();
    array_list_l_append(list, MAGIC_TEST_VALUE);
    array_list_l_insert(list, 0, 0);

    assert(0 == array_list_l_get(list, 0));
    assert(MAGIC_TEST_VALUE == array_list_l_get(list, 1));

    array_list_l_destroy(list);
}

void
test_array_list_l_take(void)
{
    array_list_l* list = al_l_create();
    array_list_l_append(list, MAGIC_TEST_VALUE);

    long num = array_list_l_take(list, 0);
    assert(num == MAGIC_TEST_VALUE);
    array_list_l_destroy(list);
}

// Used to check for memory leaks
void
test_array_list_l_destroy(void)
{
    array_list_l* lst = al_l_create();
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        array_list_l_append(lst, (long)i);
    }
    array_list_l_destroy(lst);
}

void
test_al_node_create(void)
{
    array_list_node* list = al_node_create();

    assert(list);
    assert(list->len == 0);
    assert(list->alloced == 128);

    array_list_node_destroy(list);
}

void
test_array_list_node_size(void)
{
    array_list_node* list = al_node_create();

    assert(list->len == array_list_node_size(list));

    node_t* value = new_node();
    array_list_node_append(list, value);

    assert(list->len == array_list_node_size(list));
    assert(1 == array_list_node_size(list));

    array_list_node_destroy(list);
}

void
test_array_list_node_append(void)
{
    array_list_node* list = al_node_create();
    assert(0 == array_list_node_size(list));

    node_t* val = new_node();
    val->id     = 0;
    array_list_node_append(list, val);
    assert(1 == array_list_node_size(list));

    node_t* val1 = new_node();
    val1->id     = 1;
    array_list_node_append(list, val1);
    assert(2 == array_list_node_size(list));

    assert(node_equals(list->elements[1], val1));

    array_list_node_destroy(list);
}

void
test_array_list_node_insert(void)
{
    array_list_node* list = al_node_create();

    assert(0 == array_list_node_size(list));

    node_t* val = new_node();
    val->id     = 0;
    array_list_node_append(list, val);

    assert(1 == array_list_node_size(list));

    node_t* val1 = new_node();
    val1->id     = 1;
    array_list_node_insert(list, val1, 0);
    assert(2 == array_list_node_size(list));
    assert(node_equals(list->elements[0], val1));

    array_list_node_destroy(list);
}

void
test_array_list_node_remove(void)
{
    array_list_node* list = al_node_create();

    node_t* val  = new_node();
    node_t* val1 = new_node();
    val->id      = 0;
    val1->id     = 1;

    array_list_node_append(list, val);
    array_list_node_insert(list, val1, 0);
    assert(2 == array_list_node_size(list));

    array_list_node_remove(list, 0);
    assert(1 == array_list_node_size(list));

    assert(node_equals(list->elements[0], val));

    array_list_node_remove(list, 0);
    assert(0 == array_list_node_size(list));

    array_list_node_destroy(list);
}

void
test_array_list_node_remove_elem(void)
{
    array_list_node* list = al_node_create();

    node_t* val  = new_node();
    node_t* val1 = new_node();
    val->id      = 0;
    val1->id     = 1;

    array_list_node_append(list, val);
    array_list_node_insert(list, val1, 0);
    assert(2 == array_list_node_size(list));

    array_list_node_remove_elem(list, val1);
    assert(1 == array_list_node_size(list));

    assert(node_equals(list->elements[0], val));

    array_list_node_remove_elem(list, val);
    assert(0 == array_list_node_size(list));

    array_list_node_destroy(list);
}

void
test_array_list_node_index_of(void)
{

    array_list_node* list = al_node_create();

    node_t* val     = new_node();
    node_t* val1    = new_node();
    node_t* another = new_node();
    val->id         = 0;
    val1->id        = 1;
    another->id     = 2;
    array_list_node_append(list, val);
    array_list_node_insert(list, val1, 0);

    for (size_t i = 0; i < array_list_node_size(list); ++i) {
        node_pretty_print(array_list_node_get(list, i));
    }

    unsigned long index = ULONG_MAX;
    assert(0 == array_list_node_index_of(list, val, &index));
    assert(1 == index);

    assert(-1 == array_list_node_index_of(list, another, &index));

    free(another);
    array_list_node_destroy(list);
}

void
test_array_list_node_contains(void)
{
    array_list_node* list = al_node_create();

    node_t* val     = new_node();
    node_t* val1    = new_node();
    node_t* another = new_node();
    val->id         = 0;
    val1->id        = 1;
    another->id     = 2;
    array_list_node_append(list, val);
    array_list_node_insert(list, val1, 0);

    assert(array_list_node_contains(list, val));
    assert(!array_list_node_contains(list, another));

    free(another);
    array_list_node_destroy(list);
}

void
test_array_list_node_get(void)
{
    array_list_node* list = al_node_create();

    node_t* val  = new_node();
    node_t* val1 = new_node();
    val->id      = 0;
    val1->id     = 1;
    array_list_node_append(list, val);
    array_list_node_insert(list, val1, 0);

    assert(node_equals(val1, array_list_node_get(list, 0)));
    assert(node_equals(val, array_list_node_get(list, 1)));

    array_list_node_destroy(list);
}

void
test_array_list_node_take(void)
{
    array_list_node* list = al_node_create();
    node_t*          val  = new_node();
    val->id               = 0;
    array_list_node_append(list, val);

    node_t* num = array_list_node_take(list, 0);
    assert(node_equals(num, val));
    free(num);
    array_list_node_destroy(list);
}

// Used to check for memory leaks
void
test_array_list_node_destroy(void)
{
    array_list_node* lst = al_node_create();
    node_t*          n;
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        n     = new_node();
        n->id = i;
        array_list_node_append(lst, n);
    }

    array_list_node_destroy(lst);
}

void
test_al_rel_create(void)
{
    array_list_relationship* list = al_rel_create();

    assert(list);
    assert(list->len == 0);
    assert(list->alloced == 128);

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_size(void)
{
    array_list_relationship* list = al_rel_create();

    assert(list->len == array_list_relationship_size(list));

    relationship_t* value = new_relationship();
    array_list_relationship_append(list, value);

    assert(list->len == array_list_relationship_size(list));
    assert(1 == array_list_relationship_size(list));

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_append(void)
{
    array_list_relationship* list = al_rel_create();

    assert(0 == array_list_relationship_size(list));

    relationship_t* val = new_relationship();
    val->id             = 0;
    array_list_relationship_append(list, val);

    assert(1 == array_list_relationship_size(list));

    relationship_t* val1 = new_relationship();
    val1->id             = 1;
    array_list_relationship_append(list, val1);

    for (size_t i = 0; i < array_list_relationship_size(list); ++i) {
        relationship_pretty_print(array_list_relationship_get(list, i));
    }

    assert(2 == array_list_relationship_size(list));
    assert(relationship_equals(list->elements[1], val1));

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_insert(void)
{
    array_list_relationship* list = al_rel_create();

    assert(0 == array_list_relationship_size(list));

    relationship_t* val = new_relationship();
    val->id             = 0;
    array_list_relationship_append(list, val);

    assert(1 == array_list_relationship_size(list));

    relationship_t* val1 = new_relationship();
    val1->id             = 1;
    array_list_relationship_insert(list, val1, 0);
    assert(2 == array_list_relationship_size(list));
    assert(relationship_equals(list->elements[0], val1));

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_remove(void)
{
    array_list_relationship* list = al_rel_create();

    relationship_t* val  = new_relationship();
    relationship_t* val1 = new_relationship();
    val->id              = 0;
    val1->id             = 1;

    array_list_relationship_append(list, val);
    array_list_relationship_insert(list, val1, 0);
    assert(2 == array_list_relationship_size(list));

    array_list_relationship_remove(list, 0);
    assert(1 == array_list_relationship_size(list));

    assert(relationship_equals(list->elements[0], val));

    array_list_relationship_remove(list, 0);
    assert(0 == array_list_relationship_size(list));

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_remove_elem(void)
{
    array_list_relationship* list = al_rel_create();

    relationship_t* val  = new_relationship();
    relationship_t* val1 = new_relationship();
    val->id              = 0;
    val1->id             = 1;

    array_list_relationship_append(list, val);
    array_list_relationship_insert(list, val1, 0);
    assert(2 == array_list_relationship_size(list));

    array_list_relationship_remove_elem(list, val1);
    assert(1 == array_list_relationship_size(list));

    assert(relationship_equals(list->elements[0], val));

    array_list_relationship_remove_elem(list, val);
    assert(0 == array_list_relationship_size(list));

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_index_of(void)
{

    array_list_relationship* list = al_rel_create();

    relationship_t* val     = new_relationship();
    relationship_t* val1    = new_relationship();
    relationship_t* another = new_relationship();
    val->id                 = 0;
    val1->id                = 1;
    another->id             = 2;
    array_list_relationship_append(list, val);
    array_list_relationship_insert(list, val1, 0);

    unsigned long index = ULONG_MAX;
    assert(0 == array_list_relationship_index_of(list, val, &index));
    assert(1 == index);

    assert(-1 == array_list_relationship_index_of(list, another, &index));

    free(another);
    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_contains(void)
{
    array_list_relationship* list = al_rel_create();

    relationship_t* val     = new_relationship();
    relationship_t* val1    = new_relationship();
    relationship_t* another = new_relationship();
    val->id                 = 0;
    val1->id                = 1;
    another->id             = 2;
    array_list_relationship_append(list, val);
    array_list_relationship_insert(list, val1, 0);

    assert(array_list_relationship_contains(list, val));
    assert(!array_list_relationship_contains(list, another));

    free(another);
    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_get(void)
{
    array_list_relationship* list = al_rel_create();

    relationship_t* val  = new_relationship();
    relationship_t* val1 = new_relationship();
    val->id              = 0;
    val1->id             = 1;
    array_list_relationship_append(list, val);
    array_list_relationship_insert(list, val1, 0);

    assert(relationship_equals(val1, array_list_relationship_get(list, 0)));
    assert(relationship_equals(val, array_list_relationship_get(list, 1)));

    array_list_relationship_destroy(list);
}

void
test_array_list_relationship_take(void)
{
    array_list_relationship* list = al_rel_create();
    relationship_t*          val  = new_relationship();
    val->id                       = 0;
    array_list_relationship_append(list, val);

    relationship_t* num = array_list_relationship_take(list, 0);
    assert(relationship_equals(num, val));
    free(num);
    array_list_relationship_destroy(list);
}

// Used to check for memory leaks
void
test_array_list_relationship_destroy(void)
{
    array_list_relationship* lst = al_rel_create();
    relationship_t*          n;
    for (size_t i = 0; i < MAGIC_TEST_VALUE; ++i) {
        n     = new_relationship();
        n->id = i;
        array_list_relationship_append(lst, n);
    }

    array_list_relationship_destroy(lst);
}

int
main(void)
{
    test_al_ul_create();
    test_array_list_ul_size();
    test_array_list_ul_append();
    test_array_list_ul_insert();
    test_array_list_ul_remove();
    test_array_list_ul_remove_elem();
    test_array_list_ul_index_of();
    test_array_list_ul_contains();
    test_array_list_ul_get();
    test_array_list_ul_take();
    test_array_list_ul_destroy();

    test_al_l_create();
    test_array_list_l_size();
    test_array_list_l_append();
    test_array_list_l_insert();
    test_array_list_l_remove();
    test_array_list_l_remove_elem();
    test_array_list_l_index_of();
    test_array_list_l_contains();
    test_array_list_l_get();
    test_array_list_l_take();
    test_array_list_l_destroy();

    test_al_node_create();
    test_array_list_node_size();
    test_array_list_node_append();
    test_array_list_node_insert();
    test_array_list_node_remove();
    test_array_list_node_remove_elem();
    test_array_list_node_index_of();
    test_array_list_node_contains();
    test_array_list_node_get();
    test_array_list_node_take();
    test_array_list_node_destroy();

    test_al_rel_create();
    test_array_list_relationship_size();
    test_array_list_relationship_append();
    test_array_list_relationship_insert();
    test_array_list_relationship_remove();
    test_array_list_relationship_remove_elem();
    test_array_list_relationship_index_of();
    test_array_list_relationship_contains();
    test_array_list_relationship_get();
    test_array_list_relationship_take();
    test_array_list_relationship_destroy();
}
