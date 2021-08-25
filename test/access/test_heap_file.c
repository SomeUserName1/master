#include "access/heap_file.h"

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "page_cache.h"
#include "physical_database.h"

#include <assert.h>

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

    allocate_pages(pdb, node_file, 2);

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

    allocate_pages(pdb, node_file, 2);

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

    allocate_pages(pdb, node_file, 2);

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
    assert(hf->last_alloc_node_slot == NUM_SLOTS_PER_NODE);

    page*   p    = pin_page(pc, 0, node_file);
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
    assert(hf->last_alloc_node_slot == 2 * NUM_SLOTS_PER_NODE);

    node_t* node_1 = new_node();
    node_1->id     = id_1;
    node_read(node_1, p);
    assert(node_1->id == id_1);
    assert(node_1->first_relationship == UNINITIALIZED_LONG);
    assert(node_1->label[0] == '\0');
    free(node_1);
    unpin_page(pc, 0, node_file);

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

    allocate_pages(pdb, node_file, 1);
    allocate_pages(pdb, relationship_file, 1);

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
    assert(hf->last_alloc_node_slot == 2 * NUM_SLOTS_PER_NODE);
    assert(hf->last_alloc_rel_slot == NUM_SLOTS_PER_REL);

    page*           p   = pin_page(pc, 0, relationship_file);
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

    // TODO Continue here: add more rels and check if the chaining works
    // properly

    unpin_page(pc, 0, node_file);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_read_node(void)
{}

void
test_read_relationship(void)
{}

void
test_update_node(void)
{}

void
test_update_relationship(void)
{}

void
test_delete_node(void)
{}

void
test_delete_relationship(void)
{}

void
test_move_node(void)
{}

void
test_move_relationship(void)
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
    test_heap_file_destroy();
    test_create_node();
    test_create_relationship();
    test_read_node();
    test_read_relationship();
    test_update_node();
    test_update_relationship();
    test_delete_node();
    test_delete_relationship();
    test_move_node();
    test_move_relationship();
    test_swap_page();
    test_get_nodes();
    test_get_relationships();
    test_next_relationship_id();
    test_expand();
    test_contains_relationship_from_to();

    return 0;
}
