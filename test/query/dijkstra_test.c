/*
 * dijkstra_test.c   1.0   Sep 15, 2021
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

#define n(x) dict_ul_ul_get_direct(map[0], x)
#define r(x) dict_ul_ul_get_direct(map[1], x)

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

    const char* log_path =
          "/home/someusername/workspace_local/dijkstra_test.txt";
    FILE* log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    dict_ul_ul** map =
          import_from_txt(hf,
                          "/home/someusername/workspace_local/celegans.txt",
                          false,
                          C_ELEGANS);

    sssp_result* result = dijkstra(hf, n(11), BOTH, true, log_file

    );

    assert(result->source == n(11));
    assert(dict_ul_d_get_direct(result->distances, n(11)) == 0);

    assert(dict_ul_d_get_direct(result->distances, n(0)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(0)) == r(0));

    assert(dict_ul_d_get_direct(result->distances, n(6)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(6)) == r(48));

    assert(dict_ul_d_get_direct(result->distances, n(8)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(8)) == r(83));

    assert(dict_ul_d_get_direct(result->distances, n(14)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(14)) == r(84));

    assert(dict_ul_d_get_direct(result->distances, n(20)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(20)) == r(85));

    assert(dict_ul_d_get_direct(result->distances, n(35)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(35)) == r(86));

    assert(dict_ul_d_get_direct(result->distances, n(71)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(71)) == r(87));

    assert(dict_ul_d_get_direct(result->distances, n(78)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(78)) == r(88));

    assert(dict_ul_d_get_direct(result->distances, n(79)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(79)) == r(89));

    assert(dict_ul_d_get_direct(result->distances, n(112)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(112)) == r(90));

    assert(dict_ul_d_get_direct(result->distances, n(114)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(114)) == r(91));

    assert(dict_ul_d_get_direct(result->distances, n(41)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(41)) == r(234));

    assert(dict_ul_d_get_direct(result->distances, n(43)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(43)) == r(247));

    assert(dict_ul_d_get_direct(result->distances, n(83)) == 1);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(83)) == r(587));

    assert(dict_ul_d_get_direct(result->distances, n(26)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(26)) == r(1));

    assert(dict_ul_d_get_direct(result->distances, n(69)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(69)) == r(2));

    assert(dict_ul_d_get_direct(result->distances, n(18)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(18)) == r(122));

    assert(dict_ul_d_get_direct(result->distances, n(2)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(2)) == r(15));

    assert(dict_ul_d_get_direct(result->distances, n(15)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(15)) == r(49));

    assert(dict_ul_d_get_direct(result->distances, n(16)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(16)) == r(50));

    assert(dict_ul_d_get_direct(result->distances, n(24)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(24)) == r(51));

    assert(dict_ul_d_get_direct(result->distances, n(46)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(46)) == r(52));

    assert(dict_ul_d_get_direct(result->distances, n(10)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(10)) == r(76));

    assert(dict_ul_d_get_direct(result->distances, n(45)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(45)) == r(258));

    assert(dict_ul_d_get_direct(result->distances, n(3)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(3)) == r(32));

    assert(dict_ul_d_get_direct(result->distances, n(7)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(7)) == r(54));

    assert(dict_ul_d_get_direct(result->distances, n(29)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(29)) == r(59));

    assert(dict_ul_d_get_direct(result->distances, n(31)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(31)) == r(60));

    assert(dict_ul_d_get_direct(result->distances, n(34)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(34)) == r(61));

    assert(dict_ul_d_get_direct(result->distances, n(73)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(73)) == r(63));

    assert(dict_ul_d_get_direct(result->distances, n(80)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(80)) == r(65));

    assert(dict_ul_d_get_direct(result->distances, n(100)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(100)) == r(66));

    assert(dict_ul_d_get_direct(result->distances, n(116)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(116)) == r(67));

    assert(dict_ul_d_get_direct(result->distances, n(12)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(12)) == r(93));

    assert(dict_ul_d_get_direct(result->distances, n(17)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(17)) == r(121));

    assert(dict_ul_d_get_direct(result->distances, n(19)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(19)) == r(135));

    assert(dict_ul_d_get_direct(result->distances, n(25)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(25)) == r(158));

    assert(dict_ul_d_get_direct(result->distances, n(97)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(97)) == r(635));

    assert(dict_ul_d_get_direct(result->distances, n(101)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(101)) == r(642));

    assert(dict_ul_d_get_direct(result->distances, n(9)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(9)) == r(69));

    assert(dict_ul_d_get_direct(result->distances, n(1)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(1)) == r(539));

    assert(dict_ul_d_get_direct(result->distances, n(72)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(72)) == r(479));

    assert(dict_ul_d_get_direct(result->distances, n(21)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(21)) == r(149));

    assert(dict_ul_d_get_direct(result->distances, n(42)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(42)) == r(244));

    assert(dict_ul_d_get_direct(result->distances, n(13)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(13)) == r(101));

    assert(dict_ul_d_get_direct(result->distances, n(27)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(27)) == r(542));

    assert(dict_ul_d_get_direct(result->distances, n(30)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(30)) == r(563));

    assert(dict_ul_d_get_direct(result->distances, n(33)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(33)) == r(198));

    assert(dict_ul_d_get_direct(result->distances, n(38)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(38)) == r(223));

    assert(dict_ul_d_get_direct(result->distances, n(40)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(40)) == r(232));

    assert(dict_ul_d_get_direct(result->distances, n(47)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(47)) == r(269));

    assert(dict_ul_d_get_direct(result->distances, n(49)) == 2);
    assert(dict_ul_ul_get_direct(result->pred_edges, n(49)) == r(544));

    sssp_result_destroy(result);
    dict_ul_ul_destroy(map[0]);
    dict_ul_ul_destroy(map[1]);
    free(map);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    fclose(log_file);
}
