#include "access/heap_file.h"

#include "access/header_page.h"
#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "page_cache.h"
#include "physical_database.h"

#include <assert.h>
#include <limits.h>

static const double        test_weight_1  = 2.0;
static const double        test_weight_2  = 3.0;
static const double        test_weight_3  = 4.0;
static const double        test_weight_4  = 5.0;
static const unsigned char next_slot_test = 6;
static const unsigned long num_pages_for_two_header_p =
      1 + PAGE_SIZE * CHAR_BIT / SLOTS_PER_PAGE;

void
test_heap_file_create(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 2);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    assert(hf);
    assert(hf->cache == pc);
    assert(hf->last_alloc_node_slot == 0);
    assert(hf->last_alloc_rel_slot == 0);
    assert(hf->n_nodes == 0);
    assert(hf->n_rels == 0);
    assert(hf->num_reads_nodes == 0);
    assert(hf->num_updates_nodes == 0);
    assert(hf->num_reads_rels == 0);
    assert(hf->num_update_rels == 0);

    free(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_heap_file_destroy(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 2);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    assert(hf);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_next_free_slots(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long node_slot = next_free_slots(hf, true);
    assert(node_slot == 0);
    assert(hf->last_alloc_node_slot == 0);

    unsigned char* write_mask = malloc(sizeof(unsigned char));
    write_mask[0]             = UCHAR_MAX;
    page* p                   = pin_page(pc, 0, header, node_ft);
    write_bits(pc, p, 0, 0, 3, write_mask);

    node_slot = next_free_slots(hf, true);
    assert(node_slot == NUM_SLOTS_PER_NODE);
    assert(hf->last_alloc_node_slot == 0);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, 3, 3, write_mask);

    node_slot = next_free_slots(hf, true);
    assert(node_slot == 2 * NUM_SLOTS_PER_NODE);
    assert(hf->last_alloc_node_slot == 1);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, next_slot_test, 3, write_mask);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = 1;
    write_bits(pc, p, 1, 3, 1, write_mask);

    node_slot = next_free_slots(hf, true);
    assert(node_slot == 4 * NUM_SLOTS_PER_NODE);
    assert(hf->last_alloc_node_slot == 1);

    memset(p->data, UCHAR_MAX, PAGE_SIZE);

    allocate_pages(pdb, node_ft, num_pages_for_two_header_p);
    node_slot = next_free_slots(hf, true);

    assert(hf->last_alloc_node_slot == PAGE_SIZE);
    assert(node_slot == 32768);

    page* p2 = pin_page(pc, 1, header, node_ft);
    memset(p2->data, UCHAR_MAX, PAGE_SIZE);

    assert(pdb->records[node_ft]->num_pages == num_pages_for_two_header_p + 1);

    node_slot = next_free_slots(hf, true);

    assert(hf->last_alloc_node_slot == 2 * PAGE_SIZE);
    /* as we havent allocated sufficient record pages to
                             actually be able to fill the second header page we
       get a node slot of 132 pages * 256 slots per page */

    assert(node_slot == 33280);
    assert(pdb->records[node_ft]->num_pages == num_pages_for_two_header_p + 2);

    unpin_page(pc, 1, header, node_ft);
    unpin_page(pc, 0, header, node_ft);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_check_record_exists(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long node_slot = next_free_slots(hf, true);

    assert(check_record_exists(hf, node_slot / NUM_SLOTS_PER_NODE, true));
}

void
test_create_node(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long id = create_node(hf, "\0");

    assert(id != UNINITIALIZED_LONG);
    assert(id == 0);
    assert(hf->n_nodes == 1);
    assert(hf->last_alloc_node_slot == NUM_SLOTS_PER_NODE / CHAR_BIT);
    assert(hf->num_updates_nodes == 1);

    page*   p    = pin_page(pc, 0, records, node_ft);
    node_t* node = new_node();
    node->id     = id;
    node_read(node, p);
    assert(node->id == id);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label[0] == '\0');
    free(node);

    unsigned long id_1 = create_node(hf, "\0");

    assert(id_1 != UNINITIALIZED_LONG);
    assert(id == 1);
    assert(hf->n_nodes == 2);
    assert(hf->last_alloc_node_slot == 2 * NUM_SLOTS_PER_NODE / CHAR_BIT);
    assert(hf->num_updates_nodes == 2);

    node_t* node_1 = new_node();
    node_1->id     = id_1;
    node_read(node_1, p);
    assert(node_1->id == id_1);
    assert(node_1->first_relationship == UNINITIALIZED_LONG);
    assert(node_1->label[0] == '\0');
    free(node_1);
    unpin_page(pc, 0, records, node_ft);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_create_relationship(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);
    allocate_pages(pdb, relationship_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long n_1 = create_node(hf, "\0");
    unsigned long n_2 = create_node(hf, "\0");
    unsigned long id  = create_relationship(hf, n_1, n_2, 1.0, "\0");

    assert(id != UNINITIALIZED_LONG);
    assert(id == 0);
    assert(hf->n_nodes == 2);
    assert(hf->n_rels == 1);
    assert(hf->last_alloc_node_slot == 2 * NUM_SLOTS_PER_NODE / CHAR_BIT);
    assert(hf->last_alloc_rel_slot == NUM_SLOTS_PER_REL / CHAR_BIT);

    page*           p   = pin_page(pc, 0, records, relationship_ft);
    relationship_t* rel = new_relationship();
    rel->id             = id;
    relationship_read(rel, p);
    assert(rel->id == id);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_2);
    assert(rel->prev_rel_source == id);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id);
    assert(rel->next_rel_target == id);
    assert(rel->flags == 3);
    assert(rel->weight == 1.0);
    assert(rel->label[0] == '\0');
    free(rel);

    page*   node_p = pin_page(pc, 0, records, node_ft);
    node_t* node_1 = new_node();
    node_1->id     = n_1;
    node_read(node_1, node_p);
    assert(node_1->id == n_1);
    assert(node_1->first_relationship == id);
    node_t* node_2 = new_node();
    node_2->id     = n_2;
    node_read(node_2, node_p);
    assert(node_2->id == n_2);
    assert(node_2->first_relationship == id);
    free(node_1);
    free(node_2);

    unsigned long n_3  = create_node(hf, "\0");
    unsigned long n_4  = create_node(hf, "\0");
    unsigned long id_2 = create_relationship(hf, n_1, n_3, test_weight_1, "\0");
    unsigned long id_3 = create_relationship(hf, n_1, n_4, test_weight_2, "\0");
    unsigned long id_4 = create_relationship(hf, n_2, n_3, test_weight_3, "\0");
    unsigned long id_5 = create_relationship(hf, n_2, n_4, test_weight_4, "\0");

    assert(hf->n_nodes == 4);
    assert(hf->n_rels == 5);
    assert(hf->last_alloc_node_slot == 4 * NUM_SLOTS_PER_NODE / CHAR_BIT);
    assert(hf->last_alloc_rel_slot == 5 * NUM_SLOTS_PER_REL / CHAR_BIT);

    rel     = new_relationship();
    rel->id = id;
    relationship_read(rel, p);
    assert(rel->id == id);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_2);
    assert(rel->prev_rel_source == id_3);
    assert(rel->next_rel_source == id_2);
    assert(rel->prev_rel_target == id_5);
    assert(rel->next_rel_target == id_4);
    assert(rel->flags == 3);
    assert(rel->weight == 1.0);
    assert(rel->label[0] == '\0');
    free(rel);
    node_1     = new_node();
    node_1->id = n_1;
    node_read(node_1, node_p);
    assert(node_1->id == n_1);
    assert(node_1->first_relationship == id);
    node_2     = new_node();
    node_2->id = n_2;
    node_read(node_2, node_p);
    assert(node_2->id == n_2);
    assert(node_2->first_relationship == id);
    free(node_1);
    free(node_2);

    rel     = new_relationship();
    rel->id = id_2;
    relationship_read(rel, p);
    assert(rel->id == id_2);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_3);
    assert(rel->prev_rel_source == id);
    assert(rel->next_rel_source == id_3);
    assert(rel->prev_rel_target == id_4);
    assert(rel->next_rel_target == id_4);
    assert(rel->flags == 2);
    assert(rel->weight == test_weight_1);
    assert(rel->label[0] == '\0');
    free(rel);

    rel     = new_relationship();
    rel->id = id_3;
    relationship_read(rel, p);
    assert(rel->id == id_3);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_4);
    assert(rel->prev_rel_source == id_2);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id_5);
    assert(rel->next_rel_target == id_5);
    assert(rel->flags == 2);
    assert(rel->weight == test_weight_2);
    assert(rel->label[0] == '\0');
    free(rel);

    node_t* node_3 = new_node();
    node_3->id     = n_3;
    node_read(node_3, node_p);
    assert(node_3->id == n_3);
    assert(node_3->first_relationship == id_2);
    node_t* node_4 = new_node();
    node_4->id     = n_4;
    node_read(node_4, node_p);
    assert(node_4->id == n_4);
    assert(node_4->first_relationship == id_3);
    free(node_3);
    free(node_4);

    rel     = new_relationship();
    rel->id = id_4;
    relationship_read(rel, p);
    assert(rel->id == id_4);
    assert(rel->source_node == n_2);
    assert(rel->target_node == n_3);
    assert(rel->prev_rel_source == id);
    assert(rel->next_rel_source == id_5);
    assert(rel->prev_rel_target == id_2);
    assert(rel->next_rel_target == id_2);
    assert(rel->flags == 0);
    assert(rel->weight == test_weight_3);
    assert(rel->label[0] == '\0');
    free(rel);

    rel     = new_relationship();
    rel->id = id_5;
    relationship_read(rel, p);
    assert(rel->id == id_5);
    assert(rel->source_node == n_2);
    assert(rel->target_node == n_4);
    assert(rel->prev_rel_source == id_5);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id_3);
    assert(rel->next_rel_target == id_3);
    assert(rel->flags == 0);
    assert(rel->weight == test_weight_4);
    assert(rel->label[0] == '\0');
    free(rel);

    unpin_page(pc, 0, records, node_ft);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_read_node(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 2);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long id = create_node(hf, "\0");

    assert(id != UNINITIALIZED_LONG);
    assert(id == 0);
    assert(hf->n_nodes == 1);
    assert(hf->last_alloc_node_slot == NUM_SLOTS_PER_NODE / CHAR_BIT);

    node_t* node = read_node(hf, id);
    assert(node->id == id);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label[0] == '\0');
    free(node);

    unsigned long id_1 = create_node(hf, "\0");

    assert(id_1 != UNINITIALIZED_LONG);
    assert(id == 1);
    assert(hf->n_nodes == 2);
    assert(hf->last_alloc_node_slot == 2 * NUM_SLOTS_PER_NODE / CHAR_BIT);

    node_t* node_1 = read_node(hf, id_1);
    assert(node_1->id == id_1);
    assert(node_1->first_relationship == UNINITIALIZED_LONG);
    assert(node_1->label[0] == '\0');
    free(node_1);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_read_relationship(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);
    allocate_pages(pdb, relationship_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long n_1 = create_node(hf, "\0");
    unsigned long n_2 = create_node(hf, "\0");
    unsigned long id  = create_relationship(hf, n_1, n_2, 1.0, "\0");

    relationship_t* rel = read_relationship(hf, id);
    assert(rel->id == id);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_2);
    assert(rel->prev_rel_source == id);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id);
    assert(rel->next_rel_target == id);
    assert(rel->flags == 3);
    assert(rel->weight == 1.0);
    assert(rel->label[0] == '\0');
    free(rel);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_update_node(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 2);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long id = create_node(hf, "\0");

    node_t* node = read_node(hf, id);
    assert(node->id == id);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label[0] == '\0');

    node->first_relationship = 0;
    strncpy(node->label, "abc", 4);
    update_node(hf, node);

    assert(node->first_relationship == 0);
    assert(strcmp(node->label, "abc") == 0);
    free(node);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_update_relationship(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long n_1 = create_node(hf, "\0");
    unsigned long n_2 = create_node(hf, "\0");
    unsigned long id  = create_relationship(hf, n_1, n_2, 1.0, "\0");

    relationship_t* rel  = read_relationship(hf, id);
    rel->source_node     = 1;
    rel->target_node     = 0;
    rel->prev_rel_source = 2;
    rel->next_rel_source = 3;
    rel->prev_rel_target = 4;
    rel->next_rel_target = 1;
    strncpy(rel->label, "abc", 4);
    update_relationship(hf, rel);

    assert(rel->source_node == 1);
    assert(rel->target_node == 0);
    assert(rel->prev_rel_source == 2);
    assert(rel->next_rel_source == 3);
    assert(rel->prev_rel_target == 4);
    assert(rel->next_rel_target == 1);

    assert(strcmp(rel->label, "abc") == 0);
    free(rel);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_delete_node(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long n_1 = create_node(hf, "\0");
    unsigned long n_2 = create_node(hf, "\0");

    assert(check_record_exists(hf, n_1, true));
    assert(check_record_exists(hf, n_2, true));
    delete_node(hf, n_1);
    assert(!check_record_exists(hf, n_1, true));
    assert(check_record_exists(hf, n_2, true));

    delete_node(hf, n_2);

    assert(!check_record_exists(hf, n_2, true));
    assert(!check_record_exists(hf, n_1, true));

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_delete_relationship(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );

    allocate_pages(pdb, node_ft, 1);

    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    unsigned long n_1 = create_node(hf, "\0");
    unsigned long n_2 = create_node(hf, "\0");
    unsigned long rel = create_relationship(hf, n_1, n_2, test_weight_1, "\0");

    assert(check_record_exists(hf, n_1, false));
    delete_relationship(hf, n_1);
    assert(!check_record_exists(hf, rel, false));

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_prepare_move_node(void)
{}

void
test_prepare_move_relationship(void)
{}

void
test_swap_page(void)
{}

void
test_get_nodes(void)
{}

void
test_get_relationships(void)
{}

void
test_next_relationship_id(void)
{}

void
test_expand(void)
{}

void
test_contains_relationship_from_to(void)
{}

int
main(void)
{
    test_heap_file_create();
    printf("finished test create heap file\n");
    test_heap_file_destroy();
    printf("finished test destroy heap file\n");
    test_next_free_slots();
    printf("finished test next free slot\n");
    test_check_record_exists();
    printf("finished test record exists\n");
    test_create_node();
    printf("finished test create node\n");
    test_create_relationship();
    printf("finished test create relationship\n");
    test_read_node();
    printf("finished test read node\n");
    test_read_relationship();
    printf("finished test read rel\n");
    test_update_node();
    printf("finished test update_node\n");
    test_update_relationship();
    printf("finished test update_relationship\n");
    test_delete_node();
    printf("finished test delete_node\n");
    test_delete_relationship();
    printf("finished test delete_relationship\n");
    test_prepare_move_node();
    printf("finished test prepare_move_node\n");
    test_prepare_move_relationship();
    printf("finished test prepare_move_relationship\n");
    test_swap_page();
    printf("finished test swap_page\n");
    test_get_nodes();
    printf("finished test get_nodes\n");
    test_get_relationships();
    printf("finished test get_rels\n");
    test_next_relationship_id();
    printf("finished test next rel id\n");
    test_expand();
    printf("finished test expand\n");
    test_contains_relationship_from_to();
    printf("finished test contains rel\n");

    return 0;
}
