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

    const size_t record0 = 0;
    const size_t record1 = 1;
    const size_t record2 = 2;
    const size_t record3 = 3;
    const size_t record4 = 4;
    const size_t record5 = 5;
    const size_t record6 = 6;
    const size_t record7 = 7;
    const size_t record8 = 8;
    const size_t record9 = 9;
    size_t       nids[NUM_NODES];
    for (size_t i = 0; i < NUM_NODES; ++i) {
        nids[i] = create_node(hf, 0, false);
    }

    const size_t nrels = 9;
    size_t       rids[nrels];
    rids[record0] = create_relationship(hf, record0, record1, 1.0, 0, false);
    rids[record1] = create_relationship(hf, record0, record2, 1.0, 0, false);
    rids[record2] = create_relationship(hf, record0, record3, 1.0, 0, false);

    rids[record3] = create_relationship(hf, record1, record4, 1.0, 0, false);
    rids[record4] = create_relationship(hf, record1, record5, 1.0, 0, false);

    rids[record5] = create_relationship(hf, record2, record6, 1.0, 0, false);
    rids[record6] = create_relationship(hf, record2, record7, 1.0, 0, false);

    rids[record7] = create_relationship(hf, record3, record8, 1.0, 0, false);
    rids[record8] = create_relationship(hf, record3, record9, 1.0, 0, false);

    const char* log_path = "/home/someusername/workspace_local/dfs_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    traversal_result* result = dfs(hf, 0, BOTH, true, log_file);

    assert(result->source == 0);
    assert(dict_ul_ul_get_direct(result->traversal_numbers, nids[0]) == 0);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, nids[1]) == 1);
    assert(dict_ul_ul_get_direct(result->parents, nids[1]) == rids[0]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, nids[2]) == 1);
    assert(dict_ul_ul_get_direct(result->parents, nids[2]) == rids[1]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, nids[3]) == 1);
    assert(dict_ul_ul_get_direct(result->parents, nids[3]) == rids[2]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, nids[8]) == 2);
    assert(dict_ul_ul_get_direct(result->parents, nids[8]) == rids[7]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, nids[9]) == 2);
    assert(dict_ul_ul_get_direct(result->parents, nids[9]) == rids[8]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 6) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 6) == rids[5]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 7) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 7) == rids[6]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 4) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 4) == rids[3]);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 5) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 5) == rids[4]);

    traversal_result_destroy(result);

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    fclose(log_file);
}
