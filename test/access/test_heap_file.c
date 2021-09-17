/*
 * @(#)test_heap_file.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
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
    assert(hf->last_alloc_node_id == 0);
    assert(hf->last_alloc_rel_id == 0);
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

    unsigned char* write_mask = malloc(sizeof(unsigned char));
    write_mask[0]             = UCHAR_MAX;
    page* p                   = pin_page(pc, 0, header, node_ft);
    write_bits(pc, p, 0, 0, 3, write_mask);

    assert(check_record_exists(hf, 0, true));

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, 3, 3, write_mask);

    assert(check_record_exists(hf, 0, true));
    assert(check_record_exists(hf, 1, true));
    assert(!check_record_exists(hf, 2, true));
    assert(!check_record_exists(hf, 3, true));
    assert(!check_record_exists(hf, 85, true));

    unpin_page(pc, 0, header, node_ft);
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

    next_free_slots(hf, true);
    assert(hf->last_alloc_node_id == 0);

    unsigned char* write_mask = malloc(sizeof(unsigned char));
    write_mask[0]             = UCHAR_MAX;
    page* p                   = pin_page(pc, 0, header, node_ft);
    write_bits(pc, p, 0, 0, 3, write_mask);

    next_free_slots(hf, true);
    assert(hf->last_alloc_node_id == 1);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, 3, 3, write_mask);

    next_free_slots(hf, true);
    assert(hf->last_alloc_node_id == 2);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, next_slot_test, 3, write_mask);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 1, 1, 3, write_mask);

    next_free_slots(hf, true);
    assert(hf->last_alloc_node_id == 4);

    memset(p->data, UCHAR_MAX, PAGE_SIZE);
    allocate_pages(pdb, node_ft, num_pages_for_two_header_p);

    next_free_slots(hf, true);
    assert(hf->last_alloc_node_id
           == PAGE_SIZE * CHAR_BIT / NUM_SLOTS_PER_NODE + 1);
    unpin_page(pc, 0, header, node_ft);

    page* p2 = pin_page(pc, 1, header, node_ft);
    assert(p2->pin_count > 0);
    memset(p2->data, UCHAR_MAX, PAGE_SIZE);
    unpin_page(pc, 1, header, node_ft);

    assert(pdb->records[node_ft]->num_pages == num_pages_for_two_header_p + 1);

    next_free_slots(hf, true);
    assert(hf->last_alloc_node_id
           == (pdb->records[node_ft]->num_pages - 1) * SLOTS_PER_PAGE
                          / NUM_SLOTS_PER_NODE
                    + (pdb->records[node_ft]->num_pages
                             - 1 * SLOTS_PER_PAGE / NUM_SLOTS_PER_NODE
                       != 0));
    assert(pdb->records[node_ft]->num_pages == num_pages_for_two_header_p + 2);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
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
    assert(hf->last_alloc_node_id == 0);
    assert(hf->num_updates_nodes == 1);
    assert(check_record_exists(hf, id, true));

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
    assert(id_1 == 1);
    assert(hf->n_nodes == 2);
    assert(hf->last_alloc_node_id == 1);
    assert(hf->num_updates_nodes == 2);
    assert(check_record_exists(hf, id_1, true));

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
    assert(hf->last_alloc_node_id == 1);
    assert(hf->last_alloc_rel_id == 0);

    relationship_t* rel = new_relationship();

    rel->id = id;
    page* p = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
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

    node_t* node_1 = new_node();
    node_t* node_2 = new_node();

    node_1->id   = n_1;
    page* node_p = pin_page(pc, 0, records, node_ft);
    node_read(node_1, node_p);
    unpin_page(pc, 0, records, node_ft);
    assert(node_1->id == n_1);
    assert(node_1->first_relationship == id);

    node_2->id = n_2;
    node_p     = pin_page(pc, 0, records, node_ft);
    node_read(node_2, node_p);
    unpin_page(pc, 0, records, node_ft);
    assert(node_2->id == n_2);
    assert(node_2->first_relationship == id);
    free(node_1);
    free(node_2);

    unsigned long n_3  = create_node(hf, "\0");
    unsigned long n_4  = create_node(hf, "\0");
    unsigned long id_2 = create_relationship(hf, n_1, n_3, test_weight_1, "\0");

    rel     = new_relationship();
    rel->id = id_2;

    p = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);

    assert(rel->id == id_2);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_3);
    assert(rel->prev_rel_source == id);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id_2);
    assert(rel->next_rel_target == id_2);
    assert(rel->flags == 2);
    assert(rel->weight == test_weight_1);
    assert(rel->label[0] == '\0');
    free(rel);

    rel     = new_relationship();
    rel->id = id;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
    assert(rel->id == id);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_2);
    assert(rel->prev_rel_source == id_2);
    assert(rel->next_rel_source == id_2);
    assert(rel->prev_rel_target == id);
    assert(rel->next_rel_target == id);
    assert(rel->flags == 3);
    assert(rel->weight == 1.0);
    assert(rel->label[0] == '\0');
    free(rel);

    unsigned long id_3 = create_relationship(hf, n_1, n_4, test_weight_2, "\0");

    rel     = new_relationship();
    rel->id = id_3;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
    assert(rel->id == id_3);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_4);
    assert(rel->prev_rel_source == id_2);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id_3);
    assert(rel->next_rel_target == id_3);
    assert(rel->flags == 2);
    assert(rel->weight == test_weight_2);
    assert(rel->label[0] == '\0');
    free(rel);

    rel     = new_relationship();
    rel->id = id_2;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
    assert(rel->id == id_2);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_3);
    assert(rel->prev_rel_source == id);
    assert(rel->next_rel_source == id_3);
    assert(rel->prev_rel_target == id_2);
    assert(rel->next_rel_target == id_2);
    assert(rel->flags == 2);
    assert(rel->weight == test_weight_1);
    assert(rel->label[0] == '\0');
    free(rel);

    rel     = new_relationship();
    rel->id = id;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
    assert(rel->id == id);
    assert(rel->source_node == n_1);
    assert(rel->target_node == n_2);
    assert(rel->prev_rel_source == id_3);
    assert(rel->next_rel_source == id_2);
    assert(rel->prev_rel_target == id);
    assert(rel->next_rel_target == id);
    assert(rel->flags == 3);
    assert(rel->weight == 1.0);
    assert(rel->label[0] == '\0');
    free(rel);

    unsigned long id_4 = create_relationship(hf, n_2, n_3, test_weight_3, "\0");
    unsigned long id_5 = create_relationship(hf, n_2, n_4, test_weight_4, "\0");

    assert(hf->n_nodes == 4);
    assert(hf->n_rels == 5);
    assert(hf->last_alloc_node_id == 3);
    assert(hf->last_alloc_rel_id == 4);

    node_1     = new_node();
    node_1->id = n_1;
    node_p     = pin_page(pc, 0, records, node_ft);
    node_read(node_1, node_p);
    unpin_page(pc, 0, records, node_ft);
    assert(node_1->id == n_1);
    assert(node_1->first_relationship == id);

    node_2     = new_node();
    node_2->id = n_2;
    node_p     = pin_page(pc, 0, records, node_ft);
    node_read(node_2, node_p);
    unpin_page(pc, 0, records, node_ft);
    assert(node_2->id == n_2);
    assert(node_2->first_relationship == id);

    node_t* node_3 = new_node();
    node_3->id     = n_3;
    node_p         = pin_page(pc, 0, records, node_ft);
    node_read(node_3, node_p);
    unpin_page(pc, 0, records, node_ft);
    assert(node_3->id == n_3);
    assert(node_3->first_relationship == id_2);

    node_t* node_4 = new_node();
    node_4->id     = n_4;
    node_p         = pin_page(pc, 0, records, node_ft);
    node_read(node_4, node_p);
    unpin_page(pc, 0, records, node_ft);
    assert(node_4->id == n_4);
    assert(node_4->first_relationship == id_3);

    free(node_1);
    free(node_2);
    free(node_3);
    free(node_4);

    rel     = new_relationship();
    rel->id = id;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
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

    rel     = new_relationship();
    rel->id = id_2;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
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
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
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

    rel     = new_relationship();
    rel->id = id_4;
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
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
    p       = pin_page(pc, 0, records, relationship_ft);
    relationship_read(rel, p);
    unpin_page(pc, 0, records, relationship_ft);
    assert(rel->id == id_5);
    assert(rel->source_node == n_2);
    assert(rel->target_node == n_4);
    assert(rel->prev_rel_source == id_4);
    assert(rel->next_rel_source == id);
    assert(rel->prev_rel_target == id_3);
    assert(rel->next_rel_target == id_3);
    assert(rel->flags == 0);
    assert(rel->weight == test_weight_4);
    assert(rel->label[0] == '\0');
    free(rel);

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
    assert(hf->last_alloc_node_id == 0);

    node_t* node = read_node(hf, id);
    assert(node->id == id);
    assert(node->first_relationship == UNINITIALIZED_LONG);
    assert(node->label[0] == '\0');
    free(node);

    unsigned long id_1 = create_node(hf, "\0");

    assert(id_1 != UNINITIALIZED_LONG);
    assert(id_1 == 1);
    assert(hf->n_nodes == 2);
    assert(hf->last_alloc_node_id == 1);

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
    unsigned long rel = create_relationship(hf, n_1, n_2, test_weight_1, "\0");

    assert(check_record_exists(hf, n_1, false));
    delete_relationship(hf, n_1);
    assert(!check_record_exists(hf, rel, false));

    node_t* n1 = read_node(hf, n_1);
    node_t* n2 = read_node(hf, n_2);
    assert(n1->first_relationship == UNINITIALIZED_LONG);
    assert(n2->first_relationship == UNINITIALIZED_LONG);
    free(n1);
    free(n2);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_get_nodes(void)
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
    unsigned long n_3 = create_node(hf, "\0");
    unsigned long n_4 = create_node(hf, "\0");
    unsigned long n_5 = create_node(hf, "\0");
    unsigned long n_6 = create_node(hf, "\0");

    delete_node(hf, n_4);

    array_list_node* nodes = get_nodes(hf);
    assert(array_list_node_size(nodes) == 5);
    assert(array_list_node_get(nodes, 0)->id == n_1);
    assert(array_list_node_get(nodes, 1)->id == n_2);
    assert(array_list_node_get(nodes, 2)->id == n_3);
    assert(array_list_node_get(nodes, 3)->id == n_5);
    assert(array_list_node_get(nodes, 4)->id == n_6);

    array_list_node_destroy(nodes);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_get_relationships(void)
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

    unsigned long r_1 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_2 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_3 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_4 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_5 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_6 = create_relationship(hf, n_1, n_1, 1.0, "\0");

    delete_relationship(hf, r_4);

    array_list_relationship* rels = get_relationships(hf);
    assert(array_list_relationship_size(rels) == 5);
    assert(array_list_relationship_get(rels, 0)->id == r_1);
    assert(array_list_relationship_get(rels, 1)->id == r_2);
    assert(array_list_relationship_get(rels, 2)->id == r_3);
    assert(array_list_relationship_get(rels, 3)->id == r_5);
    assert(array_list_relationship_get(rels, 4)->id == r_6);

    array_list_relationship_destroy(rels);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_rel_chain_small(void)
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

    unsigned long n_5 = create_node(hf, "\0");
    unsigned long n_6 = create_node(hf, "\0");

    unsigned long r_9 = create_relationship(hf, n_5, n_5, 1.0, "\0");
    unsigned long r_7 = create_relationship(hf, n_5, n_6, 1.0, "\0");
    unsigned long r_8 = create_relationship(hf, n_6, n_5, 1.0, "\0");

    relationship_t* r9 = read_relationship(hf, r_9);
    relationship_t* r7 = read_relationship(hf, r_7);
    relationship_t* r8 = read_relationship(hf, r_8);

    assert(r9->next_rel_source == r_7);
    assert(r9->prev_rel_source == r_8);
    assert(r9->next_rel_target == r_7);
    assert(r9->prev_rel_target == r_8);

    assert(r7->next_rel_source == r_8);
    assert(r7->prev_rel_source == r_9);
    assert(r7->next_rel_target == r_8);
    assert(r7->prev_rel_target == r_8);

    assert(r8->next_rel_source == r_7);
    assert(r8->prev_rel_source == r_7);
    assert(r8->next_rel_target == r_9);
    assert(r8->prev_rel_target == r_7);

    unsigned long n_1 = create_node(hf, "\0");
    unsigned long n_2 = create_node(hf, "\0");
    unsigned long n_3 = create_node(hf, "\0");

    unsigned long r_1 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_2 = create_relationship(hf, n_1, n_2, 1.0, "\0");
    unsigned long r_3 = create_relationship(hf, n_1, n_3, 1.0, "\0");
    unsigned long r_5 = create_relationship(hf, n_3, n_1, 1.0, "\0");
    unsigned long r_6 = create_relationship(hf, n_2, n_1, 1.0, "\0");

    relationship_t* r1 = read_relationship(hf, r_1);
    relationship_t* r2 = read_relationship(hf, r_2);
    relationship_t* r3 = read_relationship(hf, r_3);
    relationship_t* r5 = read_relationship(hf, r_5);
    relationship_t* r6 = read_relationship(hf, r_6);

    assert(r1->next_rel_source == r_2);
    assert(r1->prev_rel_source == r_6);
    assert(r1->next_rel_target == r_2);
    assert(r1->prev_rel_target == r_6);

    assert(r2->next_rel_source == r_3);
    assert(r2->prev_rel_source == r_1);
    assert(r2->next_rel_target == r_6);
    assert(r2->prev_rel_target == r_6);

    assert(r3->next_rel_source == r_5);
    assert(r3->prev_rel_source == r_2);
    assert(r3->next_rel_target == r_5);
    assert(r3->prev_rel_target == r_5);

    assert(r5->next_rel_source == r_3);
    assert(r5->prev_rel_source == r_3);
    assert(r5->next_rel_target == r_6);
    assert(r5->prev_rel_target == r_3);

    assert(r6->next_rel_source == r_2);
    assert(r6->prev_rel_source == r_2);
    assert(r6->next_rel_target == r_1);
    assert(r6->prev_rel_target == r_5);

    free(r1);
    free(r2);
    free(r3);
    free(r5);
    free(r6);
    free(r7);
    free(r8);
    free(r9);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_next_relationship_id(void)
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
    unsigned long n_3 = create_node(hf, "\0");

    unsigned long r_1 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_2 = create_relationship(hf, n_1, n_2, 1.0, "\0");
    unsigned long r_3 = create_relationship(hf, n_1, n_3, 1.0, "\0");
    unsigned long r_5 = create_relationship(hf, n_3, n_1, 1.0, "\0");

    relationship_t* r1 = read_relationship(hf, r_1);
    relationship_t* r2 = read_relationship(hf, r_2);
    relationship_t* r3 = read_relationship(hf, r_3);
    relationship_t* r5 = read_relationship(hf, r_5);

    assert(next_relationship_id(hf, n_1, r1, BOTH) == r_2);
    assert(next_relationship_id(hf, n_1, r2, BOTH) == r_3);
    assert(next_relationship_id(hf, n_1, r3, BOTH) == r_5);
    assert(next_relationship_id(hf, n_1, r5, BOTH) == r_1);
    assert(next_relationship_id(hf, n_1, r1, OUTGOING) == r_2);
    assert(next_relationship_id(hf, n_1, r2, OUTGOING) == r_3);
    assert(next_relationship_id(hf, n_1, r3, OUTGOING) == r_1);
    assert(next_relationship_id(hf, n_1, r1, INCOMING) == r_5);
    assert(next_relationship_id(hf, n_1, r5, INCOMING) == r_1);

    free(r1);
    free(r2);
    free(r3);
    free(r5);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_expand(void)
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
    unsigned long n_3 = create_node(hf, "\0");

    unsigned long r_1 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_2 = create_relationship(hf, n_1, n_2, 1.0, "\0");
    unsigned long r_3 = create_relationship(hf, n_1, n_3, 1.0, "\0");
    unsigned long r_5 = create_relationship(hf, n_3, n_1, 1.0, "\0");

    relationship_t* r1 = read_relationship(hf, r_1);
    relationship_t* r2 = read_relationship(hf, r_2);
    relationship_t* r3 = read_relationship(hf, r_3);
    relationship_t* r5 = read_relationship(hf, r_5);

    array_list_relationship* rels  = expand(hf, n_1, BOTH);
    array_list_relationship* rels1 = expand(hf, n_1, INCOMING);
    array_list_relationship* rels2 = expand(hf, n_1, OUTGOING);
    array_list_relationship* rels4 = expand(hf, n_2, BOTH);
    array_list_relationship* rels5 = expand(hf, n_2, OUTGOING);
    array_list_relationship* rels6 = expand(hf, n_2, INCOMING);
    array_list_relationship* rels7 = expand(hf, n_3, BOTH);
    array_list_relationship* rels8 = expand(hf, n_3, OUTGOING);
    array_list_relationship* rels9 = expand(hf, n_3, INCOMING);

    assert(array_list_relationship_size(rels) == 4);
    assert(array_list_relationship_size(rels1) == 2);
    assert(array_list_relationship_size(rels2) == 3);
    assert(array_list_relationship_size(rels4) == 1);
    assert(array_list_relationship_size(rels5) == 0);
    assert(array_list_relationship_size(rels6) == 1);
    assert(array_list_relationship_size(rels7) == 2);
    assert(array_list_relationship_size(rels8) == 1);
    assert(array_list_relationship_size(rels9) == 1);

    free(r1);
    free(r2);
    free(r3);
    free(r5);

    array_list_relationship_destroy(rels);
    array_list_relationship_destroy(rels1);
    array_list_relationship_destroy(rels2);
    array_list_relationship_destroy(rels4);
    array_list_relationship_destroy(rels5);
    array_list_relationship_destroy(rels6);
    array_list_relationship_destroy(rels7);
    array_list_relationship_destroy(rels8);
    array_list_relationship_destroy(rels9);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_contains_relationship_from_to(void)
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

    unsigned long              n_1    = create_node(hf, "\0");
    unsigned long              n_2    = create_node(hf, "\0");
    unsigned long              n_3    = create_node(hf, "\0");
    static const unsigned long non_ex = 666;

    unsigned long r_1 = create_relationship(hf, n_1, n_1, 1.0, "\0");
    unsigned long r_2 = create_relationship(hf, n_1, n_2, 1.0, "\0");
    unsigned long r_3 = create_relationship(hf, n_1, n_3, 1.0, "\0");
    unsigned long r_5 = create_relationship(hf, n_3, n_1, 1.0, "\0");

    relationship_t* r1  = contains_relationship_from_to(hf, n_1, n_1, BOTH);
    relationship_t* r11 = contains_relationship_from_to(hf, n_1, n_1, INCOMING);
    relationship_t* r12 = contains_relationship_from_to(hf, n_1, n_1, OUTGOING);

    relationship_t* r2  = contains_relationship_from_to(hf, n_1, n_2, BOTH);
    relationship_t* r21 = contains_relationship_from_to(hf, n_1, n_2, INCOMING);
    relationship_t* r22 = contains_relationship_from_to(hf, n_1, n_2, OUTGOING);

    relationship_t* r3  = contains_relationship_from_to(hf, n_1, n_3, BOTH);
    relationship_t* r31 = contains_relationship_from_to(hf, n_1, n_3, INCOMING);
    relationship_t* r32 = contains_relationship_from_to(hf, n_1, n_3, OUTGOING);

    relationship_t* r5  = contains_relationship_from_to(hf, n_3, n_1, BOTH);
    relationship_t* r51 = contains_relationship_from_to(hf, n_3, n_1, INCOMING);
    relationship_t* r52 = contains_relationship_from_to(hf, n_3, n_1, OUTGOING);

    relationship_t* r6  = contains_relationship_from_to(hf, non_ex, n_1, BOTH);
    relationship_t* r61 = contains_relationship_from_to(hf, n_2, n_3, BOTH);

    assert(r1->id == r_1);
    assert(r11->id == r_1);
    assert(r12->id == r_1);

    assert(r2->id == r_2);
    assert(r21 == NULL);
    assert(r22->id == r_2);

    assert(r3->id == r_3);
    assert(r31->id == r_5);
    assert(r32->id == r_3);

    assert(r5->id == r_5 || r5->id == r_3);
    assert(r51->id == r_3);
    assert(r52->id == r_5);

    assert(r6 == NULL);
    assert(r61 == NULL);

    free(r1);
    free(r11);
    free(r12);
    free(r2);
    free(r22);
    free(r3);
    free(r31);
    free(r32);
    free(r5);
    free(r51);
    free(r52);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

int
main(void)
{
    test_check_record_exists();
    printf("finished test record exists\n");
    test_heap_file_create();
    printf("finished test create heap file\n");
    test_heap_file_destroy();
    printf("finished test destroy heap file\n");
    test_next_free_slots();
    printf("finished test next free slot\n");
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
    test_get_nodes();
    printf("finished test get_nodes\n");
    test_get_relationships();
    printf("finished test get_rels\n");
    test_rel_chain_small();
    printf("finished test rel chain small\n");
    test_next_relationship_id();
    printf("finished test next rel id\n");
    test_expand();
    printf("finished test expand\n");
    test_contains_relationship_from_to();
    printf("finished test contains rel\n");

    return 0;
}
