/*
 * @(#)dijkstra_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/dijkstra.h"

#include <assert.h>
#include <errno.h>

#include "access/heap_file.h"
#include "data-struct/htable.h"
#include "query/result_types.h"
#include "query/snap_importer.h"

#define n(x) dict_ul_ul_get_direct(map, x)

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

    dict_ul_ul* map =
          import_from_txt(hf,
                          "/home/someusername/workspace_local/celegans.txt",
                          false,
                          C_ELEGANS);

    sssp_result* result = dijkstra(hf,
                                   n(11),
                                   BOTH
#ifdef VERBOSE
                                   ,
                                   log_file
#endif
    );

    assert(result->source == n(11));
    assert(dict_ul_d_get_direct(result->distances, n(11)) == 0);

    assert(dict_ul_d_get_direct(result->distances, n(0)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(0)) == 0);

    assert(dict_ul_d_get_direct(result->distances, n(6)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(6)) == 48);

    assert(dict_ul_d_get_direct(result->distances, n(8)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(8)) == 83);

    assert(dict_ul_d_get_direct(result->distances, n(14)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(14)) == 84);

    assert(dict_ul_d_get_direct(result->distances, n(20)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(20)) == 85);

    assert(dict_ul_d_get_direct(result->distances, n(35)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(35)) == 86);

    assert(dict_ul_d_get_direct(result->distances, n(71)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(71)) == 87);

    assert(dict_ul_d_get_direct(result->distances, n(78)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(78)) == 88);

    assert(dict_ul_d_get_direct(result->distances, n(79)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(79)) == 89);

    assert(dict_ul_d_get_direct(result->distances, n(112)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(112)) == 90);

    assert(dict_ul_d_get_direct(result->distances, n(114)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(114)) == 91);

    assert(dict_ul_d_get_direct(result->distances, n(41)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(41)) == 234);

    assert(dict_ul_d_get_direct(result->distances, n(43)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(43)) == 247);

    assert(dict_ul_d_get_direct(result->distances, n(83)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(83)) == 587);

    assert(dict_ul_d_get_direct(result->distances, n(26)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(26)) == 1);

    assert(dict_ul_d_get_direct(result->distances, n(69)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(69)) == 2);

    assert(dict_ul_d_get_direct(result->distances, n(18)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(18)) == 122);

    assert(dict_ul_d_get_direct(result->distances, n(2)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(2)) == 15);

    assert(dict_ul_d_get_direct(result->distances, n(15)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(15)) == 49);

    assert(dict_ul_d_get_direct(result->distances, n(16)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(16)) == 50);

    assert(dict_ul_d_get_direct(result->distances, n(24)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(24)) == 51);

    assert(dict_ul_d_get_direct(result->distances, n(46)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(46)) == 52);

    assert(dict_ul_d_get_direct(result->distances, n(10)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(10)) == 76);

    assert(dict_ul_d_get_direct(result->distances, n(45)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(45)) == 258);

    assert(dict_ul_d_get_direct(result->distances, n(3)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(3)) == 32);

    assert(dict_ul_d_get_direct(result->distances, n(7)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(7)) == 54);

    assert(dict_ul_d_get_direct(result->distances, n(29)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(29)) == 59);

    assert(dict_ul_d_get_direct(result->distances, n(31)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(31)) == 60);

    assert(dict_ul_d_get_direct(result->distances, n(34)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(34)) == 61);

    assert(dict_ul_d_get_direct(result->distances, n(73)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(73)) == 63);

    assert(dict_ul_d_get_direct(result->distances, n(80)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(80)) == 65);

    assert(dict_ul_d_get_direct(result->distances, n(100)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(100)) == 66);

    assert(dict_ul_d_get_direct(result->distances, n(116)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(116)) == 67);

    assert(dict_ul_d_get_direct(result->distances, n(12)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(12)) == 93);

    assert(dict_ul_d_get_direct(result->distances, n(17)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(17)) == 121);

    assert(dict_ul_d_get_direct(result->distances, n(19)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(19)) == 135);

    assert(dict_ul_d_get_direct(result->distances, n(25)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(25)) == 158);

    assert(dict_ul_d_get_direct(result->distances, n(97)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(97)) == 635);

    assert(dict_ul_d_get_direct(result->distances, n(101)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(101)) == 642);

    assert(dict_ul_d_get_direct(result->distances, n(9)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(9)) == 69);

    assert(dict_ul_d_get_direct(result->distances, n(1)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(1)) == 539);

    assert(dict_ul_d_get_direct(result->distances, n(72)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(72)) == 479);

    assert(dict_ul_d_get_direct(result->distances, n(21)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(21)) == 149);

    assert(dict_ul_d_get_direct(result->distances, n(42)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(42)) == 244);

    assert(dict_ul_d_get_direct(result->distances, n(13)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(13)) == 101);

    assert(dict_ul_d_get_direct(result->distances, n(27)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(27)) == 542);

    assert(dict_ul_d_get_direct(result->distances, n(30)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(30)) == 563);

    assert(dict_ul_d_get_direct(result->distances, n(33)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(33)) == 198);

    assert(dict_ul_d_get_direct(result->distances, n(38)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(38)) == 223);

    assert(dict_ul_d_get_direct(result->distances, n(40)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(40)) == 232);

    assert(dict_ul_d_get_direct(result->distances, n(47)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(47)) == 269);

    assert(dict_ul_d_get_direct(result->distances, n(49)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(49)) == 544);

    sssp_result_destroy(result);
    dict_ul_ul_destroy(map);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

#ifdef VERBOSE
    fclose(log_file);
#endif
}
