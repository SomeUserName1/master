/*
 * @(#)random_layout_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */

#include "access/node.h"
#include "layout/random_layout.h"

#include <assert.h>

#include "access/heap_file.h"
#include "data-struct/htable.h"
#include "query/snap_importer.h"

heap_file*
prepare(void)
{
#ifdef VERBOSE
    log_file = fopen(log_path, "w+");
    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_phf   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* phf = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_phf
#endif
    );

    page_cache* pc = page_cache_create(phf,
                                       CACHE_N_PAGES
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

#ifdef VERBOSE
    fclose(log_file);
#endif
}

void
test_identity_order(void)
{
    heap_file* hf = prepare();

    dict_ul_ul* order = identity_order(hf);

    assert(dict_ul_ul_size(order) == hf->n_nodes);

    array_list_node* nodes = get_nodes(hf);

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        assert(dict_ul_ul_contains(order, array_list_node_get(nodes, i)->id));
    }

    clean_up(hf);
}

void
test_random_order(void)
{
    heap_file* hf = prepare();

    dict_ul_ul* order = random_order(hf);

    assert(dict_ul_ul_size(order) == hf->n_nodes);

    array_list_node* nodes = get_nodes(hf);

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        assert(dict_ul_ul_contains(order, array_list_node_get(nodes, i)->id));
    }

    clean_up(hf);
}

int
main(void)
{
    test_identity_order();
    printf("random layout test - test identity_order successful\n");
    test_random_order();
    printf("random layout test - test random_order successful\n");

    return 0;
}
