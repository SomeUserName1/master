/*
 * a-star_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "access/relationship.h"
#include "query/a-star.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "access/heap_file.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "query/result_types.h"
#include "query/snap_importer.h"

#define n(x) dict_ul_ul_get_direct(map[0], x)
#define r(x) dict_ul_ul_get_direct(map[1], x)

int
main(void)
{
    char* file_name = "test";

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";

    phy_database* pdb = phy_database_create(file_name, log_name_pdb);

    page_cache* pc = page_cache_create(pdb, CACHE_N_PAGES, log_name_cache);

    heap_file* hf = heap_file_create(pc, log_name_file);

    dict_ul_ul** map =
          import_from_txt(hf,
                          "/home/someusername/workspace_local/celegans.txt",
                          false,
                          C_ELEGANS);

    dict_ul_d* heuristic = d_ul_d_create();

    for (size_t i = 0; i < dict_ul_ul_size(map[0]); ++i) {
        dict_ul_d_insert(heuristic, n(i), 0.0);
    }

    const char* log_path = "/home/someusername/workspace_local/astar_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    path* result = a_star(hf, heuristic, n(11), n(111), BOTH, true, log_file);

    assert(result->source == n(11));
    assert(result->target == n(111));
    assert(result->distance == 3);

    relationship_t* rel;
    unsigned long   next_node_id;
    bool            src;

    assert(array_list_ul_get(result->edges, 0) == r(88));
    rel          = read_relationship(hf, r(88), false);
    src          = rel->source_node == n(11);
    next_node_id = src ? rel->target_node : rel->source_node;
    assert(src && rel->source_node == n(11) || rel->target_node == n(11));
    free(rel);

    assert(array_list_ul_get(result->edges, 1) == r(561));
    rel = read_relationship(hf, r(561), false);
    src = next_node_id == rel->source_node;
    assert(src && rel->source_node == next_node_id
           || rel->target_node == next_node_id);
    next_node_id = src ? rel->target_node : rel->source_node;
    free(rel);

    assert(array_list_ul_get(result->edges, 2) == r(763));
    rel = read_relationship(hf, r(763), false);
    src = next_node_id == rel->source_node;
    assert(src && rel->source_node == next_node_id
           || rel->target_node == next_node_id);
    assert(src && rel->target_node == n(111) || rel->source_node == n(111));
    free(rel);

    path_destroy(result);
    dict_ul_ul_destroy(map[0]);
    dict_ul_ul_destroy(map[1]);
    free(map);
    dict_ul_d_destroy(heuristic);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
    fclose(log_file);
}
