#include "../../src/constants.h"
#include "../../src/record/relationship.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SUFFICENT_BUF_SIZE (512)
#define SMALL_BUF_SIZE (32)
#define NUM (123)
#define OTHER_NUM (222)

void
test_new_rel(void)
{
    relationship_t* relationship = new_relationship();
    assert(relationship);
    free(relationship);
}

void
test_relationship_read(void)
{
    relationship_t* relationship = new_relationship();
    unsigned char a[SMALL_BUF_SIZE] = { 0 };
    assert(!relationship_read(relationship, a));
    free(relationship);
    printf("Stub only, not yet implemented!\n");
}

void
test_relationship_write(void)
{
    relationship_t* relationship = new_relationship();
    assert(!relationship_write(relationship));
    free(relationship);

    printf("Stub only, not yet implemented!\n");
}

void
test_relationship_clear(void)
{
    relationship_t* relationship = new_relationship();
    relationship->id = 1;
    relationship->flags = 1;
    relationship->source_node = 1;
    relationship->target_node = 1;
    relationship->relationship_type = 1;
    relationship->prev_rel_source = 1;
    relationship->prev_rel_target = 1;
    relationship->next_rel_source = 1;
    relationship->next_rel_target = 1;
    relationship->weight = 1.0;

    relationship_clear(relationship);

    assert(relationship->id == UNINITIALIZED_LONG);
    assert(relationship->flags == UNINITIALIZED_BYTE);
    assert(relationship->source_node == UNINITIALIZED_LONG);
    assert(relationship->target_node == UNINITIALIZED_LONG);
    assert(relationship->relationship_type == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_source == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_target == UNINITIALIZED_LONG);
    assert(relationship->next_rel_source == UNINITIALIZED_LONG);
    assert(relationship->next_rel_target == UNINITIALIZED_LONG);
    assert(relationship->weight == UNINITIALIZED_WEIGHT);

    free(relationship);
}

void
test_relationship_copy(void)
{
    relationship_t* relationship = new_relationship();
    relationship->id = 1;
    relationship->flags = 1;
    relationship->source_node = 1;
    relationship->target_node = 1;
    relationship->relationship_type = 1;
    relationship->prev_rel_source = 1;
    relationship->prev_rel_target = 1;
    relationship->next_rel_source = 1;
    relationship->next_rel_target = 1;
    relationship->weight = 1.0;

    relationship_t* copy = relationship_copy(relationship);
    assert(relationship->id == copy->id);
    assert(relationship->flags == copy->flags);
    assert(relationship->source_node == copy->source_node);
    assert(relationship->target_node == copy->target_node);
    assert(relationship->relationship_type == copy->relationship_type);
    assert(relationship->prev_rel_source == copy->prev_rel_source);
    assert(relationship->prev_rel_target == copy->prev_rel_target);
    assert(relationship->next_rel_source == copy->next_rel_source);
    assert(relationship->next_rel_target == copy->next_rel_target);
    assert(relationship->weight == copy->weight);

    copy->source_node = 2;
    assert(relationship->source_node != copy->source_node);

    free(relationship);
    free(copy);
}

void
test_relationship_equals(void)
{
    relationship_t* relationship = new_relationship();
    relationship->id = NUM;
    relationship->source_node = NUM;
    relationship_t* eq_relationship = relationship_copy(relationship);

    assert(relationship_equals(relationship, eq_relationship));

    eq_relationship->id = OTHER_NUM;

    assert(!relationship_equals(relationship, eq_relationship));

    eq_relationship->id = relationship->id;
    eq_relationship->source_node = OTHER_NUM;

    assert(!relationship_equals(relationship, eq_relationship));

    free(relationship);
    free(eq_relationship);
}

void
test_relationship_to_string(void)
{
    relationship_t* relationship = new_relationship();
    char too_small_buf[SMALL_BUF_SIZE];
    char buf[SUFFICENT_BUF_SIZE];

    assert(!relationship_to_string(relationship, buf, SUFFICENT_BUF_SIZE));
    assert(relationship_to_string(relationship, too_small_buf, SMALL_BUF_SIZE));

    assert(buf[0] == 'R');
    free(relationship);
}

void
test_relationship_pretty_print(void)
{
    relationship_t* relationship = new_relationship();

    relationship_pretty_print(relationship);
    free(relationship);
}

void
test_first_rel_flag(void)
{
    relationship_t* rel = new_relationship();

    relationship_set_first_source(rel);
    assert(rel->flags == FIRST_REL_SOURCE_FLAG);

    relationship_clear(rel);

    relationship_set_first_target(rel);
    assert(rel->flags == FIRST_REL_TARGET_FLAG);

    relationship_set_first_source(rel);
    assert(rel->flags == (FIRST_REL_SOURCE_FLAG | FIRST_REL_TARGET_FLAG));

    free(rel);
}

int
main(void)
{
    test_new_rel();
    test_relationship_read();
    test_relationship_write();
    test_relationship_clear();
    test_relationship_copy();
    test_relationship_equals();
    test_relationship_to_string();
    test_relationship_pretty_print();
    test_first_rel_flag();
    printf("Sucessfully tested relationship records!\n");
}
