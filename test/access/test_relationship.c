/*
 * test_relationship.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "access/relationship.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "page_cache.h"

#define SUFFICENT_BUF_SIZE (512)
#define SMALL_BUF_SIZE     (32)
#define NUM                (123)
#define OTHER_NUM          (222)
#define NUM_ULONG_FIELDS   (6)
static const double        TEST_DBL = 2.0;
static const unsigned long label    = 11111111;

void
test_new_rel(void)
{
    relationship_t* relationship = new_relationship();
    assert(relationship);
    assert(relationship->id == UNINITIALIZED_LONG);
    assert(relationship->source_node == UNINITIALIZED_LONG);
    assert(relationship->target_node == UNINITIALIZED_LONG);
    assert(relationship->next_rel_source == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_source == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_target == UNINITIALIZED_LONG);
    assert(relationship->next_rel_target == UNINITIALIZED_LONG);
    assert(relationship->weight == UNINITIALIZED_WEIGHT);
    assert(relationship->label == UNINITIALIZED_LONG);
    free(relationship);
}

void
test_relationship_free(void)
{
    relationship_t* rel = new_relationship();
    rel_free(rel);
}

void
test_relationship_clear(void)
{
    relationship_t* relationship  = new_relationship();
    relationship->id              = 1;
    relationship->source_node     = 1;
    relationship->target_node     = 1;
    relationship->prev_rel_source = 1;
    relationship->prev_rel_target = 1;
    relationship->next_rel_source = 1;
    relationship->next_rel_target = 1;
    relationship->weight          = 1.0;
    relationship->label           = label;

    relationship_clear(relationship);

    assert(relationship->id == UNINITIALIZED_LONG);
    assert(relationship->source_node == UNINITIALIZED_LONG);
    assert(relationship->target_node == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_source == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_target == UNINITIALIZED_LONG);
    assert(relationship->next_rel_source == UNINITIALIZED_LONG);
    assert(relationship->next_rel_target == UNINITIALIZED_LONG);
    assert(relationship->weight == UNINITIALIZED_WEIGHT);
    assert(relationship->label == UNINITIALIZED_LONG);

    free(relationship);
}

void
test_relationship_copy(void)
{
    relationship_t* relationship  = new_relationship();
    relationship->id              = 1;
    relationship->source_node     = 1;
    relationship->target_node     = 1;
    relationship->prev_rel_source = 1;
    relationship->prev_rel_target = 1;
    relationship->next_rel_source = 1;
    relationship->next_rel_target = 1;
    relationship->weight          = 1.0;
    relationship->label           = label;

    relationship_t* copy = relationship_copy(relationship);

    assert(relationship->id == copy->id);
    assert(relationship->source_node == copy->source_node);
    assert(relationship->target_node == copy->target_node);
    assert(relationship->prev_rel_source == copy->prev_rel_source);
    assert(relationship->prev_rel_target == copy->prev_rel_target);
    assert(relationship->next_rel_source == copy->next_rel_source);
    assert(relationship->next_rel_target == copy->next_rel_target);
    assert(relationship->weight == copy->weight);
    assert(relationship->label == copy->label);

    copy->source_node = 2;
    assert(relationship->source_node != copy->source_node);

    free(relationship);
    free(copy);
}

void
test_relationship_equals(void)
{
    relationship_t* relationship    = new_relationship();
    relationship->id                = NUM;
    relationship->source_node       = NUM;
    relationship_t* eq_relationship = relationship_copy(relationship);

    assert(relationship_equals(relationship, eq_relationship));

    eq_relationship->id = OTHER_NUM;

    assert(!relationship_equals(relationship, eq_relationship));

    eq_relationship->id          = relationship->id;
    eq_relationship->source_node = OTHER_NUM;

    assert(!relationship_equals(relationship, eq_relationship));

    free(relationship);
    free(eq_relationship);
}

void
test_relationship_to_string(void)
{
    relationship_t* relationship = new_relationship();
    char            buf[SUFFICENT_BUF_SIZE];

    relationship_to_string(relationship, buf, SUFFICENT_BUF_SIZE);

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
test_relationship_write(void)
{
    char* file_name = "test";

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";

    phy_database* pdb = phy_database_create(file_name, log_name_pdb);
    page_cache*   pc  = page_cache_create(pdb, CACHE_N_PAGES, log_name_cache);

    allocate_pages(pc->pdb, relationship_ft, 1, false);

    relationship_t* relationship  = new_relationship();
    relationship->id              = 3;
    relationship->source_node     = 1;
    relationship->target_node     = NUM;
    relationship->prev_rel_source = 1;
    relationship->next_rel_source = 2;
    relationship->prev_rel_target = 3;
    relationship->next_rel_target = 1;
    relationship->weight          = TEST_DBL;
    relationship->label           = label;

    page* p = pin_page(pc, 0, records, relationship_ft, false);
    relationship_write(relationship, p);

    unsigned long src =
          read_ulong(p, (relationship->id & UCHAR_MAX) * SLOT_SIZE);
    assert(src == 1);

    unsigned long trgt = read_ulong(p,
                                    (relationship->id & UCHAR_MAX) * SLOT_SIZE
                                          + sizeof(unsigned long));
    assert(trgt == NUM);

    unsigned long prv_src =
          read_ulong(p,
                     (relationship->id & UCHAR_MAX) * SLOT_SIZE
                           + sizeof(unsigned long) + sizeof(unsigned long));
    assert(prv_src == 1);

    unsigned long nxt_src = read_ulong(
          p,
          (relationship->id & UCHAR_MAX) * SLOT_SIZE + sizeof(unsigned long)
                + sizeof(unsigned long) + sizeof(unsigned long));
    assert(nxt_src == 2);

    unsigned long prv_trgt =
          read_ulong(p,
                     (relationship->id & UCHAR_MAX) * SLOT_SIZE
                           + sizeof(unsigned long) + sizeof(unsigned long)
                           + sizeof(unsigned long) + sizeof(unsigned long));
    assert(prv_trgt == 3);

    unsigned long nxt_trgt =
          read_ulong(p,
                     (relationship->id & UCHAR_MAX) * SLOT_SIZE
                           + (NUM_ULONG_FIELDS - 1) * sizeof(unsigned long));
    assert(nxt_trgt == 1);

    double weight =
          read_double(p,
                      (relationship->id & UCHAR_MAX) * SLOT_SIZE
                            + NUM_ULONG_FIELDS * sizeof(unsigned long));
    assert(weight == TEST_DBL);

    unsigned long m_label = read_ulong(
          p,
          (relationship->id & UCHAR_MAX) * SLOT_SIZE
                + NUM_ULONG_FIELDS * sizeof(unsigned long) + sizeof(double));
    assert(m_label == label);

    unpin_page(pc, 0, records, relationship_ft, false);

    free(relationship);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_relationship_read(void)
{
    char* file_name = "test";

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";

    phy_database* pdb = phy_database_create(file_name, log_name_pdb);
    page_cache*   pc  = page_cache_create(pdb, CACHE_N_PAGES, log_name_cache);

    allocate_pages(pc->pdb, relationship_ft, 1, false);

    relationship_t* relationship  = new_relationship();
    relationship->id              = 3;
    relationship->source_node     = 1;
    relationship->target_node     = NUM;
    relationship->prev_rel_source = 1;
    relationship->next_rel_source = 2;
    relationship->prev_rel_target = 3;
    relationship->next_rel_target = 1;
    relationship->weight          = TEST_DBL;
    relationship->label           = label;

    page* p = pin_page(pc, 0, records, relationship_ft, false);
    relationship_write(relationship, p);

    relationship_t* n_rel = new_relationship();
    n_rel->id             = 3;
    relationship_read(n_rel, p);
    assert(n_rel->target_node == NUM);

    unpin_page(pc, 0, records, relationship_ft, false);

    assert(relationship_equals(n_rel, relationship));

    free(relationship);
    free(n_rel);

    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

int
main(void)
{
    test_new_rel();
    test_relationship_free();
    test_relationship_clear();
    test_relationship_copy();
    test_relationship_equals();
    test_relationship_to_string();
    test_relationship_pretty_print();
    test_relationship_write();
    test_relationship_read();

    printf("Sucessfully tested relationship records!\n");
}
