/*
 * random_walk_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/random_walk.h"

#include <assert.h>
#include <errno.h>

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

    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";

    phy_database* pdb = phy_database_create(file_name

                                            ,
                                            log_name_pdb

    );

    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES

                                       ,
                                       log_name_cache

    );

    heap_file* hf = heap_file_create(pc

                                     ,
                                     log_name_file

    );

    const char* log_path = "/home/someusername/workspace_local/randw_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    import(hf, false, C_ELEGANS);

    const unsigned int max_walk_steps = 100;
    path*              rand_w;
    relationship_t*    r;
    relationship_t*    r_next;

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(hf, 0, i, BOTH, true, log_file

        );
        assert(rand_w->distance == i);
        assert(rand_w->source == 0);
        assert(array_list_ul_size(rand_w->edges) == i);

        for (size_t j = 0; j < i - 1; ++j) {
            // for direction BOTH, consecutive edges need to share one node, no
            // matter if src or target
            r = read_relationship(
                  hf, array_list_ul_get(rand_w->edges, j), false);
            r_next = read_relationship(
                  hf, array_list_ul_get(rand_w->edges, j + 1), false);
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
        rand_w = random_walk(hf, 0, i, OUTGOING, false, log_file

        );
        assert(rand_w->distance <= i);
        assert(rand_w->source == 0);
        assert(array_list_ul_size(rand_w->edges) <= i);

        for (size_t j = 0; j < (size_t)rand_w->distance - 1; ++j) {
            // for direction OUTGOING, r target must correspond to r_next's
            // source
            r = read_relationship(
                  hf, array_list_ul_get(rand_w->edges, j), false);
            r_next = read_relationship(
                  hf, array_list_ul_get(rand_w->edges, j + 1), false);
            assert(r->target_node == r_next->source_node);
            free(r);
            free(r_next);
        }

        path_destroy(rand_w);
    }

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(hf, 0, i, INCOMING, false, log_file

        );
        assert(rand_w->distance <= i);
        assert(rand_w->source == 0);
        assert(array_list_ul_size(rand_w->edges) <= i);

        for (size_t j = 0; j < (size_t)rand_w->distance - 1; ++j) {
            // for direction OUTGOING, r target must correspond to r_next's
            // source
            r = read_relationship(
                  hf, array_list_ul_get(rand_w->edges, j), false);
            r_next = read_relationship(
                  hf, array_list_ul_get(rand_w->edges, j + 1), false);
            assert(r->source_node == r_next->target_node);
            free(r);
            free(r_next);
        }

        path_destroy(rand_w);
    }

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    fclose(log_file);
}
