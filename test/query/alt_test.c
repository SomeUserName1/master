/*
 * @(#)alt_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/alt.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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

    page_cache* pc = page_cache_create(pdb,
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

    dict_ul_ul** map =
          import_from_txt(hf,
                          "/home/someusername/workspace_local/celegans.txt",
                          false,
                          C_ELEGANS);

    const unsigned long num_landmarks = 3;

#ifdef VERBOSE
    const char* log_path = "/home/someusername/workspace_local/alt_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    dict_ul_d* heuristic[num_landmarks];

    alt_preprocess(hf,
                   BOTH,
                   num_landmarks,
                   heuristic
#ifdef VERBOSE
                         log_file
#endif
    );

    path* result = alt(hf,
                       heuristic,
                       num_landmarks,
                       n(11),
                       n(111),
                       BOTH
#ifdef VERBOSE
                             log_file
#endif
    );

    assert(result->source == n(11));
    assert(result->target == n(111));
    assert(result->distance == 3);

    path_destroy(result);
    dict_ul_ul_destroy(map[0]);
    dict_ul_ul_destroy(map[1]);
    free(map);

    for (size_t i = 0; i < num_landmarks; ++i) {
        dict_ul_d_destroy(heuristic[i]);
    }

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

#ifdef VERBOSE
    fclose(log_file);
#endif
}
