/*
 * @(#)test_node.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "access/node.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"

#define SUFFICENT_BUF_SIZE (512)
#define SMALL_BUF_SIZE     (32)
#define NUM                (123)
#define OTHER_NUM          (222)

void
test_new_node(void)
{
    node_t* node = new_node();
    assert(node);
    assert(node->id == UNINITIALIZED_LONG);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label == UNINITIALIZED_LONG);

    free(node);
}

void
test_node_free(void)
{
    node_t* node = new_node();
    node_free(node);
}

void
test_node_clear(void)
{
    node_t* node              = new_node();
    node->id                  = 1;
    node->first_relationship  = 1;
    const unsigned long label = 123;
    node->label               = label;

    node_clear(node);

    assert(node->id == UNINITIALIZED_LONG);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label == UNINITIALIZED_LONG);

    free(node);
}

void
test_node_copy(void)
{
    node_t* node              = new_node();
    node->id                  = 1;
    node->first_relationship  = 1;
    const unsigned long label = 11111111;
    node->label               = label;

    node_t* copy = node_copy(node);

    assert(node->id == copy->id);
    assert(node->first_relationship == copy->first_relationship);
    assert(copy->label == label);

    free(node);
    free(copy);
}

void
test_node_equals(void)
{
    node_t* node              = new_node();
    node->id                  = NUM;
    node->first_relationship  = NUM;
    const unsigned long label = 11111111;
    node->label               = label;

    node_t* eq_node = node_copy(node);

    assert(node_equals(node, eq_node));

    eq_node->id = OTHER_NUM;

    assert(!node_equals(node, eq_node));

    eq_node->id                 = node->id;
    eq_node->first_relationship = OTHER_NUM;

    assert(!node_equals(node, eq_node));

    eq_node->first_relationship = node->first_relationship;
    eq_node->label              = 0;

    assert(!node_equals(node, eq_node));

    free(node);
    free(eq_node);
}

void
test_node_to_string(void)
{
    node_t* node = new_node();
    char    buf[SUFFICENT_BUF_SIZE];

    node_to_string(node, buf, SUFFICENT_BUF_SIZE);

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

void
test_node_write(void)
{
    char* file_name = "test";

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";

    phy_database* pdb = phy_database_create(file_name, log_name_pdb);
    page_cache*   pc  = page_cache_create(pdb, CACHE_N_PAGES, log_name_cache);

    allocate_pages(pc->pdb, node_ft, 1, false);

    node_t* node              = new_node();
    node->id                  = 1;
    node->first_relationship  = NUM;
    const unsigned long label = 11111111;
    node->label               = label;

    page* p = pin_page(pc, 0, records, node_ft, false);
    node_write(node, p);

    unsigned long fst_rel = read_ulong(p, (node->id & UCHAR_MAX) * SLOT_SIZE);
    assert(fst_rel == node->first_relationship);

    unsigned long m_label = read_ulong(
          p, (node->id & UCHAR_MAX) * SLOT_SIZE + sizeof(unsigned long));

    unpin_page(pc, 0, records, node_ft, false);

    assert(m_label == node->label);

    free(node);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_node_read(void)
{
    char* file_name = "test";

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";

    phy_database* pdb = phy_database_create(file_name, log_name_pdb);
    page_cache*   pc  = page_cache_create(pdb, CACHE_N_PAGES, log_name_cache);

    allocate_pages(pc->pdb, node_ft, 1, false);

    node_t* node              = new_node();
    node->id                  = 1;
    node->first_relationship  = NUM;
    const unsigned long label = 11111111;
    node->label               = label;
    page* p                   = pin_page(pc, 0, records, node_ft, false);
    node_write(node, p);

    node_t* n_node = new_node();
    n_node->id     = 1;
    node_read(n_node, p);

    unpin_page(pc, 0, records, node_ft, false);

    assert(node_equals(n_node, node));

    free(node);
    free(n_node);

    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

int
main(void)
{
    test_new_node();
    test_node_free();
    test_node_clear();
    test_node_copy();
    test_node_equals();
    test_node_to_string();
    test_node_pretty_print();
    test_node_write();
    test_node_read();

    printf("Sucessfully tested node records!\n");
}
