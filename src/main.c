/*!
 * \file main.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
// LCOV_EXCL_START
#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/htable.h"
#include "order/random_order.h"
#include "order/reorder_records.h"
#include "page_cache.h"
#include "physical_database.h"
#include "query/a-star.h"
#include "query/alt.h"
#include "query/bfs.h"
#include "query/dfs.h"
#include "query/dijkstra.h"
#include "query/random_walk.h"
#include "query/result_types.h"
#include "query/snap_importer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define NUM_LANDMARKS  (3)
#define PERMISSION_NUM (0777)

void
rek_mkdir(char* path)
{
    char* sep = strrchr(path, '/');
    if (sep != NULL) {
        *sep = 0;
        rek_mkdir(path);
        *sep = '/';
        if (mkdir(path, PERMISSION_NUM) && errno != EEXIST) {
            printf("error while trying to create '%s'\n%m\n", path);
        }
    }
}

int
main(void)
{
    printf("Main: Set up the database.\n");
    char* db_name        = "demo";
    char* log_name_pdb   = "pdb_before.log";
    char* log_name_cache = "pc_before.log";
    char* log_name_file  = "hf_before.log";

    const unsigned long kib_to_gib = 1 << 10;

    phy_database* pdb = phy_database_create(db_name, log_name_pdb);
    // 2 is the number of pages in the page cache
    page_cache* pc = page_cache_create(pdb, kib_to_gib, log_name_cache);
    heap_file*  hf = heap_file_create(pc, log_name_file);

    printf("Main: Import a dataset.\n");
    // This function downloads a fresh copy of the dataset on
    // every execution. If you want to import the dataset from a .txt file or if
    // you need to know the labels, use import_from_txt which also spits out a
    // map for both nodes and relationships from db ids to labels/dataset ids
    import(hf, false, EMAIL_EU_CORE);

    printf("Main: Preprare the queries.\n");
    array_list_node* nodes = get_nodes(hf, false);

    // Chose a random start node and end node for the queries
    unsigned long start_id =
          array_list_node_get(nodes, rand() % hf->n_nodes)->id;
    unsigned long end_id = array_list_node_get(nodes, rand() % hf->n_nodes)->id;

    array_list_node_destroy(nodes);

    // Preprocess the landmarks for alt
    const unsigned long n_landmarks = 3;
    dict_ul_d*          landmarks[3];

    alt_preprocess(hf, OUTGOING, n_landmarks, landmarks, false, NULL);

    // Create and open a log file to log the accesses before reordering the
    // records
    const char* query_before_log = "queries_before.log";
    FILE*       log_file         = fopen(query_before_log, "a");

    if (!log_file) {
        printf("Main: Failed to open log file %s: %s\n",
               query_before_log,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    page_cache_change_n_frames(pc, 2);

    printf("Main: Executing queries.\n");
    // Flush between queries
    traversal_result* bfs_res = bfs(hf, start_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    traversal_result* dfs_res = dfs(hf, start_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    sssp_result* dijkstra_res =
          dijkstra(hf, start_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    path* alt_res = alt(hf,
                        landmarks,
                        n_landmarks,
                        start_id,
                        end_id,
                        OUTGOING,
                        true,
                        log_file);
    flush_all_pages(pc, false);

    path* a_star_res = a_star(hf,
                              dijkstra_res->distances,
                              start_id,
                              end_id,
                              OUTGOING,
                              true,
                              log_file);
    flush_all_pages(pc, false);

    // free the results, close the log_file
    traversal_result_destroy(bfs_res);
    traversal_result_destroy(dfs_res);
    path_destroy(alt_res);
    path_destroy(a_star_res);
    sssp_result_destroy(dijkstra_res);

    if (fclose(log_file) != 0) {
        printf("Main: error closing file %s: %s\n",
               query_before_log,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Main: Finished executing queries.\n");

    // swap the log files to capture the costs for reordering
    log_name_pdb   = "pdb_reorder.log";
    log_name_cache = "pc_reorder.log";
    log_name_file  = "hf_reorder.log";

    phy_database_swap_log_file(pdb, log_name_pdb);
    page_cache_swap_log_file(pc, log_name_cache);
    heap_file_swap_log_file(hf, log_name_file);

    // Set a higher cache size for reordering
    page_cache_change_n_frames(pc, kib_to_gib);

    printf("Main: Reordering the graph on disk.\n");
    system("python "
           "/home/someusername/sync/workspace/uni/master/scripts/"
           "modularity_order.py");
    FILE* seq_file =
          fopen("/home/someusername/workspace_local/node_seq.txt", "r");
    if (!seq_file) {
        printf("Main: Error opening file %s, %s\n",
               "node_seq.txt",
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    unsigned long* seq =
          malloc(get_no_nodes(EMAIL_EU_CORE) * sizeof(unsigned long));
    for (size_t i = 0; i < get_no_nodes(EMAIL_EU_CORE); ++i) {
        fscanf(seq_file, "%lu\n", &seq[i]);
    }

    // dict_ul_ul* order = random_order(hf);
    reorder_nodes_by_sequence(hf, seq, true);
    reorder_relationships_by_nodes(hf, true);
    sort_incidence_list(hf, true);

    free(seq);

    // swap the log files to capture the queries after reordering
    log_name_pdb   = "pdb_after.log";
    log_name_cache = "pc_after.log";
    log_name_file  = "hf_after.log";

    phy_database_swap_log_file(pdb, log_name_pdb);
    page_cache_swap_log_file(pc, log_name_cache);
    heap_file_swap_log_file(hf, log_name_file);

    // Set the cache size back to 2
    page_cache_change_n_frames(pc, 2);

    // Create and open a log file
    const char* query_after_log = "queries_after.log";
    log_file                    = fopen(query_after_log, "a");

    if (!log_file) {
        printf("Main: Failed to open log file %s: %s\n",
               query_after_log,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Main: Executing queries.\n");

    bfs_res = bfs(hf, start_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    dfs_res = dfs(hf, start_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    dijkstra_res = dijkstra(hf, start_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    alt_res = alt(hf, landmarks, 3, start_id, end_id, OUTGOING, true, log_file);
    flush_all_pages(pc, false);

    a_star_res = a_star(hf,
                        dijkstra_res->distances,
                        start_id,
                        end_id,
                        OUTGOING,
                        true,
                        log_file);
    flush_all_pages(pc, false);

    // free the results, close the log_file
    traversal_result_destroy(bfs_res);
    traversal_result_destroy(dfs_res);
    path_destroy(alt_res);
    path_destroy(a_star_res);
    sssp_result_destroy(dijkstra_res);

    for (size_t i = 0; i < n_landmarks; ++i) {
        dict_ul_d_destroy(landmarks[i]);
    }

    if (fclose(log_file) != 0) {
        printf("Main: error closing file %s: %s\n",
               query_before_log,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Main: Finished executing queries.\n");

    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    return 0;
}

// LCOV_EXCL_STOP
