#include "query/bfs.h"

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

    dict_ul_ul* map = import_from_txt(
          hf, "/home/someusername/workspace_local/celegans.txt", false);

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
    assert(dict_ul_ul_get_direct(result->parents, n(0)) == 0);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(6)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(6)) == 48);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(8)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(8)) == 83);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(14)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(14)) == 84);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(20)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(20)) == 85);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(35)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(35)) == 86);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(71)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(71)) == 87);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(78)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(78)) == 88);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(79)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(79)) == 89);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(112)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(112)) == 90);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(114)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(114)) == 91);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(41)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(41)) == 234);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(43)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(43)) == 247);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(83)) == 1);
    assert(dict_ul_ul_get_direct(result->parents, n(83)) == 587);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(26)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(26)) == 1);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(69)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(69)) == 2);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(18)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(18)) == 122);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(2)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(2)) == 15);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(15)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(15)) == 49);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(16)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(16)) == 50);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(24)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(24)) == 51);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(46)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(46)) == 52);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(10)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(10)) == 76);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(45)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(45)) == 258);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(3)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(3)) == 32);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(7)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(7)) == 54);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(29)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(29)) == 59);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(31)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(31)) == 60);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(34)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(34)) == 61);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(73)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(73)) == 63);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(80)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(80)) == 65);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(100)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(100)) == 66);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(116)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(116)) == 67);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(12)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(12)) == 93);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(17)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(17)) == 121);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(19)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(19)) == 135);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(25)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(25)) == 158);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(97)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(97)) == 635);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(101)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(101)) == 642);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(9)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(9)) == 69);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(1)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(1)) == 103);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(72)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(72)) == 108);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(21)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(21)) == 149);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(42)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(42)) == 241);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(13)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(13)) == 101);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(27)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(27)) == 166);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(30)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(30)) == 182);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(33)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(33)) == 198);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(38)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(38)) == 223);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(40)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(40)) == 232);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(47)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(47)) == 269);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, n(49)) == 2);
    assert(dict_ul_ul_get_direct(result->parents, n(49)) == 278);

    traversal_result_destroy(result);
    dict_ul_ul_destroy(map);
    phy_database_delete(hf->cache->pdb);
    page_cache_destroy(hf->cache);
    heap_file_destroy(hf);
#ifdef VERBOSE
    fclose(log_file);
#endif
}
