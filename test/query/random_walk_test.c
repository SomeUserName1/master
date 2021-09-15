/*
 * @(#)random_walk_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/random_walk.h"

#include <assert.h>

#include "access/heap_file.h"
#include "access/relationship.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "query/result_types.h"
#include "query/snap_importer.h"

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

#ifdef VERBOSE
    const char* log_path = "/home/someusername/workspace_local/alt_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    dict_ul_ul_destroy(
          import_from_txt(hf,
                          "/home/someusername/workspace_local/celegans.txt",
                          false,
                          C_ELEGANS));

    const unsigned int max_walk_steps = 100;
    path*              rand_w;
    relationship_t*    r;
    relationship_t*    r_next;

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(hf,
                             0,
                             i,
                             BOTH
#ifdef VERBOSE
                             ,
                             log_file
#endif
        );
        assert(rand_w->distance == i);
        assert(rand_w->source == 0);
        assert(array_list_ul_size(rand_w->edges) == i);

        for (size_t j = 0; j < i - 1; ++j) {
            // for direction BOTH, consecutive edges need to share one node, no
            // matter if src or target
            r      = read_relationship(hf, array_list_ul_get(rand_w->edges, j));
            r_next = read_relationship(hf,
                                       array_list_ul_get(rand_w->edges, j + 1));
            assert(r->target_node == r_next->target_node
                   || r->source_node == r_next->source_node
                   || r->source_node == r_next->target_node
                   || r->target_node == r_next->source_node);

            free(r);
            free(r_next);
        }

        path_destroy(rand_w);
    }

    // The asserts below use <= instead of == as a random walk can end up with
    // no edges to take for directed random walks.

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(hf,
                             0,
                             i,
                             OUTGOING
#ifdef VERBOSE
                             ,
                             log_file
#endif
        );
        assert(rand_w->distance <= i);
        assert(rand_w->source == 0);
        assert(array_list_ul_size(rand_w->edges) <= i);

        for (size_t j = 0; j < (size_t)rand_w->distance - 1; ++j) {
            // for direction OUTGOING, r target must correspond to r_next's
            // source
            r      = read_relationship(hf, array_list_ul_get(rand_w->edges, j));
            r_next = read_relationship(hf,
                                       array_list_ul_get(rand_w->edges, j + 1));
            assert(r->target_node == r_next->source_node);
            free(r);
            free(r_next);
        }

        path_destroy(rand_w);
    }

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(hf,
                             0,
                             i,
                             INCOMING
#ifdef VERBOSE
                             ,
                             log_file
#endif
        );
        assert(rand_w->distance <= i);
        assert(rand_w->source == 0);
        assert(array_list_ul_size(rand_w->edges) <= i);

        for (size_t j = 0; j < (size_t)rand_w->distance - 1; ++j) {
            // for direction OUTGOING, r target must correspond to r_next's
            // source
            r      = read_relationship(hf, array_list_ul_get(rand_w->edges, j));
            r_next = read_relationship(hf,
                                       array_list_ul_get(rand_w->edges, j + 1));
            assert(r->source_node == r_next->target_node);
            free(r);
            free(r_next);
        }

        path_destroy(rand_w);
    }

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

#ifdef VERBOSE
    fclose(log_file);
#endif
}
