/*
 * random_layout_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */

#include "access/node.h"
#include "access/relationship.h"
#include "order/random_order.h"

#include <assert.h>
#include <errno.h>

#include "access/heap_file.h"
#include "data-struct/htable.h"
#include "query/snap_importer.h"

heap_file*
prepare(void)
{
    char* file_name      = "test";
    char* log_name_phf   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";

    phy_database* phf = phy_database_create(file_name, log_name_phf);

    page_cache* pc = page_cache_create(phf, CACHE_N_PAGES, log_name_cache);

    heap_file* hf = heap_file_create(pc, log_name_file);

    import(hf, false, C_ELEGANS);

    return hf;
}

void
clean_up(heap_file* hf)
{
    page_cache*   pc  = hf->cache;
    phy_database* pdb = pc->pdb;
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
test_identity_node_order(void)
{
    heap_file* hf = prepare();

    dict_ul_ul* order = identity_node_order(hf);

    assert(dict_ul_ul_size(order) == hf->n_nodes);

    array_list_node*     nodes = get_nodes(hf, false);
    dict_ul_ul_iterator* it    = dict_ul_ul_iterator_create(order);
    unsigned long        key   = 0;
    unsigned long        value = 0;
    unsigned long        this_value;
    unsigned long        this_key;

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        this_key = array_list_node_get(nodes, i)->id;
        assert(dict_ul_ul_contains(order, this_key));
        this_value = dict_ul_ul_get_direct(order, this_key);

        while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
            if (this_value == value && this_key != key) {
                printf("tk %lu, tv %lu, k %lu v %lu\n",
                       this_key,
                       this_value,
                       key,
                       value);
            }
            assert(this_value != value || this_key == key);
        }
        it->idx = 0;
    }
    dict_ul_ul_iterator_destroy(it);
    array_list_node_destroy(nodes);
    dict_ul_ul_destroy(order);

    clean_up(hf);
}

void
test_identity_relationship_order(void)
{
    heap_file* hf = prepare();

    dict_ul_ul* order = identity_relationship_order(hf);

    assert(dict_ul_ul_size(order) == hf->n_rels);

    array_list_relationship* rels  = get_relationships(hf, false);
    dict_ul_ul_iterator*     it    = dict_ul_ul_iterator_create(order);
    unsigned long            key   = 0;
    unsigned long            value = 0;
    unsigned long            this_value;
    unsigned long            this_key;

    for (size_t i = 0; i < hf->n_rels; ++i) {
        this_key = array_list_relationship_get(rels, i)->id;
        assert(dict_ul_ul_contains(order, this_key));

        this_value = dict_ul_ul_get_direct(order, this_key);

        while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
            assert(this_value != value || this_key == key);
        }
        it->idx = 0;
    }
    dict_ul_ul_iterator_destroy(it);
    array_list_relationship_destroy(rels);
    dict_ul_ul_destroy(order);

    clean_up(hf);
}

void
test_random_node_order(void)
{
    heap_file* hf = prepare();

    dict_ul_ul* order = random_node_order(hf);

    assert(dict_ul_ul_size(order) == hf->n_nodes);

    array_list_node*     nodes = get_nodes(hf, false);
    dict_ul_ul_iterator* it    = dict_ul_ul_iterator_create(order);
    unsigned long        key   = 0;
    unsigned long        value = 0;
    unsigned long        this_value;
    unsigned long        this_key;

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        this_key = array_list_node_get(nodes, i)->id;
        assert(dict_ul_ul_contains(order, this_key));
        this_value = dict_ul_ul_get_direct(order, this_key);

        while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
            if (this_value == value && this_key != key) {
                printf("tk %lu, tv %lu, k %lu v %lu\n",
                       this_key,
                       this_value,
                       key,
                       value);
            }
            assert(this_value != value || this_key == key);
        }
        it->idx = 0;
    }
    dict_ul_ul_iterator_destroy(it);
    array_list_node_destroy(nodes);
    dict_ul_ul_destroy(order);

    clean_up(hf);
}

void
test_random_rel_order(void)
{
    heap_file* hf = prepare();

    dict_ul_ul* order = random_relationship_order(hf);

    assert(dict_ul_ul_size(order) == hf->n_rels);

    array_list_relationship* rels  = get_relationships(hf, false);
    dict_ul_ul_iterator*     it    = dict_ul_ul_iterator_create(order);
    unsigned long            key   = 0;
    unsigned long            value = 0;
    unsigned long            this_value;
    unsigned long            this_key;

    for (size_t i = 0; i < hf->n_rels; ++i) {
        this_key = array_list_relationship_get(rels, i)->id;
        assert(dict_ul_ul_contains(order, this_key));

        this_value = dict_ul_ul_get_direct(order, this_key);

        while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
            assert(this_value != value || this_key == key);
        }
        it->idx = 0;
    }
    dict_ul_ul_iterator_destroy(it);
    array_list_relationship_destroy(rels);
    dict_ul_ul_destroy(order);

    clean_up(hf);
}

int
main(void)
{
    test_identity_node_order();
    test_identity_relationship_order();
    printf("random layout test - test identity_order successful\n");
    test_random_node_order();
    test_random_rel_order();
    printf("random layout test - test random_order successful\n");

    return 0;
}
