#include "access/relationship.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "page_cache.h"

#define SUFFICENT_BUF_SIZE (512)
#define SMALL_BUF_SIZE     (32)
#define NUM                (123)
#define OTHER_NUM          (222)
#define NUM_ULONG_FIELDS   (6)
static const double TEST_DBL = 2.0;

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
    assert(relationship->flags == UNINITIALIZED_BYTE);
    assert(relationship->weight == UNINITIALIZED_WEIGHT);
    assert(relationship->label[0] == '\0');
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
    relationship->flags           = 1;
    relationship->source_node     = 1;
    relationship->target_node     = 1;
    relationship->prev_rel_source = 1;
    relationship->prev_rel_target = 1;
    relationship->next_rel_source = 1;
    relationship->next_rel_target = 1;
    relationship->weight          = 1.0;
    memset(relationship->label, 1, MAX_STR_LEN - 1);
    relationship->label[MAX_STR_LEN - 1] = '\0';

    relationship_clear(relationship);

    assert(relationship->id == UNINITIALIZED_LONG);
    assert(relationship->flags == UNINITIALIZED_BYTE);
    assert(relationship->source_node == UNINITIALIZED_LONG);
    assert(relationship->target_node == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_source == UNINITIALIZED_LONG);
    assert(relationship->prev_rel_target == UNINITIALIZED_LONG);
    assert(relationship->next_rel_source == UNINITIALIZED_LONG);
    assert(relationship->next_rel_target == UNINITIALIZED_LONG);
    assert(relationship->weight == UNINITIALIZED_WEIGHT);
    assert(relationship->label[0] == '\0');

    free(relationship);
}

void
test_relationship_copy(void)
{
    relationship_t* relationship  = new_relationship();
    relationship->id              = 1;
    relationship->flags           = 1;
    relationship->source_node     = 1;
    relationship->target_node     = 1;
    relationship->prev_rel_source = 1;
    relationship->prev_rel_target = 1;
    relationship->next_rel_source = 1;
    relationship->next_rel_target = 1;
    relationship->weight          = 1.0;
    memset(relationship->label, 1, MAX_STR_LEN - 1);
    relationship->label[MAX_STR_LEN - 1] = '\0';

    relationship_t* copy = relationship_copy(relationship);
    assert(relationship->id == copy->id);
    assert(relationship->flags == copy->flags);
    assert(relationship->source_node == copy->source_node);
    assert(relationship->target_node == copy->target_node);
    assert(relationship->prev_rel_source == copy->prev_rel_source);
    assert(relationship->prev_rel_target == copy->prev_rel_target);
    assert(relationship->next_rel_source == copy->next_rel_source);
    assert(relationship->next_rel_target == copy->next_rel_target);
    assert(relationship->weight == copy->weight);

    for (size_t i = 0; i < MAX_STR_LEN - 1; ++i) {
        assert(relationship->label[i] == copy->label[i]);
    }

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

void
test_relationship_write(void)
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

    allocate_pages(pc->pdb, relationship_ft, 1);

    relationship_t* relationship  = new_relationship();
    relationship->id              = 3;
    relationship->source_node     = 1;
    relationship->target_node     = 2;
    relationship->prev_rel_source = 1;
    relationship->next_rel_source = 2;
    relationship->prev_rel_target = 3;
    relationship->next_rel_target = 1;
    relationship->weight          = TEST_DBL;
    relationship->flags           = 3;
    memset(relationship->label, 4, MAX_STR_LEN - 1);
    relationship->label[MAX_STR_LEN - 1] = '\0';

    page* p = pin_page(pc, 0, records, relationship_ft);
    relationship_write(relationship, p);

    unsigned long src = read_ulong(p, relationship->id * ON_DISK_REL_SIZE);
    assert(src == 1);

    unsigned long trgt = read_ulong(
          p, relationship->id * ON_DISK_REL_SIZE + sizeof(unsigned long));
    assert(trgt == 2);

    unsigned long prv_src =
          read_ulong(p,
                     relationship->id * ON_DISK_REL_SIZE + sizeof(unsigned long)
                           + sizeof(unsigned long));
    assert(prv_src == 1);

    unsigned long nxt_src =
          read_ulong(p,
                     relationship->id * ON_DISK_REL_SIZE + sizeof(unsigned long)
                           + sizeof(unsigned long) + sizeof(unsigned long));
    assert(nxt_src == 2);

    unsigned long prv_trgt =
          read_ulong(p,
                     relationship->id * ON_DISK_REL_SIZE + sizeof(unsigned long)
                           + sizeof(unsigned long) + sizeof(unsigned long)
                           + sizeof(unsigned long));
    assert(prv_trgt == 3);

    unsigned long nxt_trgt =
          read_ulong(p,
                     relationship->id * ON_DISK_REL_SIZE
                           + (NUM_ULONG_FIELDS - 1) * sizeof(unsigned long));
    assert(nxt_trgt == 1);

    double weight =
          read_double(p,
                      relationship->id * ON_DISK_REL_SIZE
                            + NUM_ULONG_FIELDS * sizeof(unsigned long));
    assert(weight == TEST_DBL);

    unsigned char flags = read_uchar(
          p,
          relationship->id * ON_DISK_REL_SIZE
                + NUM_ULONG_FIELDS * sizeof(unsigned long) + sizeof(double));
    assert(flags == 3);

    char label[MAX_STR_LEN];
    read_string(p,
                relationship->id * ON_DISK_REL_SIZE
                      + NUM_ULONG_FIELDS * sizeof(unsigned long)
                      + sizeof(double) + sizeof(unsigned char),
                label);

    unpin_page(pc, 0, records, relationship_ft);

    for (size_t i = 0; i < MAX_STR_LEN - 1; ++i) {
        assert(label[i] == 4);
    }
    assert(label[MAX_STR_LEN - 1] == relationship->label[MAX_STR_LEN - 1]);

    free(relationship);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_relationship_read(void)
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

    allocate_pages(pc->pdb, relationship_ft, 1);

    relationship_t* relationship  = new_relationship();
    relationship->id              = 3;
    relationship->source_node     = 1;
    relationship->target_node     = 2;
    relationship->prev_rel_source = 1;
    relationship->next_rel_source = 2;
    relationship->prev_rel_target = 3;
    relationship->next_rel_target = 1;
    relationship->weight          = TEST_DBL;
    relationship->flags           = 3;
    memset(relationship->label, 4, MAX_STR_LEN - 1);
    relationship->label[MAX_STR_LEN - 1] = '\0';

    page* p = pin_page(pc, 0, records, relationship_ft);
    relationship_write(relationship, p);

    relationship_t* n_rel = new_relationship();
    n_rel->id             = 3;
    relationship_read(n_rel, p);

    unpin_page(pc, 0, records, relationship_ft);

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
    test_first_rel_flag();
    test_relationship_write();
    test_relationship_read();

    printf("Sucessfully tested relationship records!\n");
}
