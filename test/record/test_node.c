#include "../../src/constants.h"
#include "../../src/record/node.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SUFFICENT_BUF_SIZE (512)
#define SMALL_BUF_SIZE (32)
#define NUM (123)
#define OTHER_NUM (222)

void
test_new_node(void)
{
    node_t* node = new_node();
    assert(node);
    free(node);
}

void
test_node_read(void)
{
    node_t* node = new_node();
    unsigned char a[SMALL_BUF_SIZE] = { 0 };
    assert(!node_read(node, a));
    free(node);
    printf("Stub only, not yet implemented!\n");
}

void
test_node_write(void)
{
    node_t* node = new_node();
    assert(!node_write(node));
    free(node);

    printf("Stub only, not yet implemented!\n");
}

void
test_node_clear(void)
{
    node_t* node = new_node();
    node->id = 1;
    node->node_type = 1;
    node->first_relationship = 1;

    node_clear(node);

    assert(node->id == UNINITIALIZED_LONG);
    assert(node->node_type == UNINITIALIZED_LONG);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->flags == UNINITIALIZED_BYTE);

    free(node);
}

void
test_node_copy(void)
{
    node_t* node = new_node();
    node->id = 1;
    node->node_type = 1;
    node->first_relationship = 1;

    node_t* copy = node_copy(node);

    assert(node->id == copy->id);
    assert(node->node_type == copy->node_type);
    assert(node->first_relationship == copy->first_relationship);

    copy->flags = 2;
    assert(node->flags != copy->flags);

    free(node);
    free(copy);
}

void
test_node_equals(void)
{
    node_t* node = new_node();
    node->id = NUM;
    node->first_relationship = NUM;
    node_t* eq_node = node_copy(node);

    assert(node_equals(node, eq_node));

    eq_node->id = OTHER_NUM;

    assert(!node_equals(node, eq_node));

    eq_node->id = node->id;
    eq_node->first_relationship = OTHER_NUM;

    assert(!node_equals(node, eq_node));

    free(node);
    free(eq_node);
}

void
test_node_to_string(void)
{
    node_t* node = new_node();
    char too_small_buf[SMALL_BUF_SIZE];
    char buf[SUFFICENT_BUF_SIZE];

    assert(!node_to_string(node, buf, SUFFICENT_BUF_SIZE));
    assert(node_to_string(node, too_small_buf, SMALL_BUF_SIZE));

    assert(buf[0] == 'N');
    free(node);
}

void
test_node_pretty_print(void)
{
    node_t* node = new_node();

    node_pretty_print(node);
    free(node);
}

int
main(void)
{
    test_new_node();
    test_node_read();
    test_node_write();
    test_node_clear();
    test_node_copy();
    test_node_equals();
    test_node_to_string();
    test_node_pretty_print();

    printf("Sucessfully tested node records!\n");
}
