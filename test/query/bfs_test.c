/*
 * @(#)bfs_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "query/bfs.h"

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

    dict_ul_ul** map =
          import_from_txt(hf,
                          "/home/someusername/workspace_local/celegans.txt",
                          false,
                          C_ELEGANS);

#ifdef VERBOSE
    const char* log_path = "/home/someusername/workspace_local/alt_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    traversal_result* result = bfs(hf,
                                   n(11),
                                   BOTH
#ifdef VERBOSE
                                   ,
                                   log_name_file
#endif
    );

    assert(result->source == n(11));
    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(11)) == 0);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(0)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(0)) == r(0));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(6)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(6)) == r(48));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(8)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(8)) == r(83));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(14)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(14)) == r(84));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(20)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(20)) == r(85));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(35)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(35)) == r(86));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(71)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(71)) == r(87));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(78)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(78)) == r(88));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(79)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(79)) == r(89));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(112)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(112)) == r(90));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(114)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(114)) == r(91));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(41)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(41)) == r(234));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(43)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(43)) == r(247));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(83)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(83)) == r(587));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(26)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(26)) == r(1));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(69)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(69)) == r(2));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(18)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(18)) == r(122));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(2)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(2)) == r(15));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(15)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(15)) == r(49));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(16)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(16)) == r(50));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(24)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(24)) == r(51));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(46)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(46)) == r(52));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(10)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(10)) == r(76));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(45)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(45)) == r(258));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(3)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(3)) == r(32));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(7)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(7)) == r(54));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(29)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(29)) == r(59));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(31)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(31)) == r(60));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(34)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(34)) == r(61));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(73)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(73)) == r(63));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(80)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(80)) == r(65));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(100)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(100)) == r(66));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(116)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(116)) == r(67));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(12)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(12)) == r(93));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(17)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(17)) == r(121));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(19)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(19)) == r(135));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(25)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(25)) == r(158));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(97)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(97)) == r(635));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(101)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(101)) == r(642));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(9)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(9)) == r(69));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(1)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(1)) == r(103));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(72)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(72)) == r(108));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(21)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(21)) == r(149));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(42)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(42)) == r(241));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(13)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(13)) == r(101));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(27)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(27)) == r(166));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(30)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(30)) == r(182));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(33)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(33)) == r(198));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(38)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(38)) == r(223));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(40)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(40)) == r(232));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(47)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(47)) == r(269));

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(49)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(49)) == r(278));

    traversal_result_destroy(result);
    dict_ul_ul_destroy(map[0]);
    dict_ul_ul_destroy(map[1]);
    free(map);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

#ifdef VERBOSE
    fclose(log_file);
#endif
}
