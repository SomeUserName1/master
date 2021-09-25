/*
 * @(#)dfs_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/dfs.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "access/heap_file.h"
#include "query/result_types.h"

#define NUM_NODES (10)

int
main(void)
{
    char* file_name = "test";

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";

    phy_database* pdb = phy_database_create(file_name, log_name_pdb);

    allocate_pages(pdb, node_ft, 1, false);
    allocate_pages(pdb, relationship_ft, 1, false);

    page_cache* pc = page_cache_create(pdb, CACHE_N_PAGES, log_name_cache);

    heap_file* hf = heap_file_create(pc, log_name_file);

    size_t nids[NUM_NODES];
    for (size_t i = 0; i < NUM_NODES; ++i) {
        nids[i] = create_node(hf, 0, false);
    }

    const size_t nrels = 9;
    size_t       rids[nrels];
    rids[0] = create_relationship(hf, nids[0], nids[1], 1.0, 0, false);
    rids[1] = create_relationship(hf, nids[0], nids[2], 1.0, 0, false);
    rids[2] = create_relationship(hf, nids[0], nids[3], 1.0, 0, false);

    rids[3] = create_relationship(hf, nids[1], nids[4], 1.0, 0, false);
    rids[4] = create_relationship(hf, nids[1], nids[5], 1.0, 0, false);

    rids[5] = create_relationship(hf, nids[2], nids[6], 1.0, 0, false);
    rids[6] = create_relationship(hf, nids[2], nids[7], 1.0, 0, false);

    rids[7] = create_relationship(hf, nids[3], nids[8], 1.0, 0, false);
    rids[8] = create_relationship(hf, nids[3], nids[9], 1.0, 0, false);

    const char* log_path = "/home/someusername/workspace_local/dfs_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    traversal_result* result = dfs(hf, 0, BOTH, true, log_file

    );

    // FIXME cont here
    assert(result->source == 0);
    assert(dict_ul_ul_get_direct(result->traversal_numbers, 0) == 0);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 1) == 1);
    assert(dict_ul_ul_get_direct(result->parents, 1) == rids[0]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 2) == 1);
    assert(dict_ul_ul_get_direct(result->parents, 2) == rids[1]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 3) == 1);
    assert(dict_ul_ul_get_direct(result->parents, 3) == rids[2]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 8) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 8) == rids[7]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 9) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 9) == 8);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 6) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 6) == 5);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 7) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 7) == 6);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 4) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 4) == 3);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 5) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 5) == 4);

    traversal_result_destroy(result);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    fclose(log_file);
}
