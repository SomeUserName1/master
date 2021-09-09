#include "access/node.h"

#include <assert.h>
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
    assert(node->label[0] == '\0');

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
    node_t* node             = new_node();
    node->id                 = 1;
    node->first_relationship = 1;
    char* str                = "ab\0";
    memcpy(node->label, str, strlen(str) + 1);

    node_clear(node);

    assert(node->id == UNINITIALIZED_LONG);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label[0] == '\0');

    free(node);
}

void
test_node_copy(void)
{
    node_t* node             = new_node();
    node->id                 = 1;
    node->first_relationship = 1;
    memset(node->label, 1, MAX_STR_LEN - 1);
    node->label[MAX_STR_LEN - 1] = '\0';

    node_t* copy = node_copy(node);

    assert(node->id == copy->id);
    assert(node->first_relationship == copy->first_relationship);

    for (size_t i = 0; i < MAX_STR_LEN - 1; ++i) {
        assert(node->label[i] == 1);
    }

    assert(node->label[MAX_STR_LEN - 1] == '\0');

    free(node);
    free(copy);
}

void
test_node_equals(void)
{
    node_t* node             = new_node();
    node->id                 = NUM;
    node->first_relationship = NUM;
    memset(node->label, 1, MAX_STR_LEN - 1);
    node->label[MAX_STR_LEN - 1] = '\0';

    node_t* eq_node = node_copy(node);

    assert(node_equals(node, eq_node));

    eq_node->id = OTHER_NUM;

    assert(!node_equals(node, eq_node));

    eq_node->id                 = node->id;
    eq_node->first_relationship = OTHER_NUM;

    assert(!node_equals(node, eq_node));

    eq_node->first_relationship = node->first_relationship;
    eq_node->label[0]           = '\0';

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

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, 1);

    node_t* node             = new_node();
    node->id                 = 1;
    node->first_relationship = NUM;
    memset(node->label, 1, MAX_STR_LEN - 1);
    node->label[MAX_STR_LEN - 1] = '\0';

    page* p = pin_page(pc, 0, records, node_ft);
    node_write(node, p);

    unsigned long fst_rel =
          read_ulong(p, node->id * NUM_SLOTS_PER_NODE * SLOT_SIZE);
    assert(fst_rel == node->first_relationship);

    char label[MAX_STR_LEN];
    read_string(p,
                node->id * NUM_SLOTS_PER_NODE * SLOT_SIZE
                      + sizeof(unsigned long),
                label);

    unpin_page(pc, 0, records, node_ft);

    for (size_t i = 0; i < MAX_STR_LEN - 1; ++i) {
        assert(label[i] == node->label[i]);
    }
    assert(label[MAX_STR_LEN - 1] == node->label[MAX_STR_LEN - 1]);

    free(node);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_node_read(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, 1);

    node_t* node             = new_node();
    node->id                 = 1;
    node->first_relationship = NUM;
    memset(node->label, 1, MAX_STR_LEN - 1);
    node->label[MAX_STR_LEN - 1] = '\0';

    page* p = pin_page(pc, 0, records, node_ft);
    node_write(node, p);

    node_t* n_node = new_node();
    n_node->id     = 1;
    node_read(n_node, p);

    unpin_page(pc, 0, records, node_ft);

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
