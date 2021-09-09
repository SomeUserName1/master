#include "query/a-star.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "access/heap_file.h"
#include "data-struct/array_list.h"
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

    dict_ul_d* heuristic = d_ul_d_create();

    for (size_t i = 0; i < dict_ul_ul_size(map); ++i) {
        dict_ul_d_insert(heuristic, n(i), 0.0);
    }

#ifdef VERBOSE
    const char* log_path = "/home/someusername/workspace_local/alt_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    path* result = a_star(hf,
                          heuristic,
                          n(11),
                          n(111),
                          BOTH
#ifdef VERBOSE
                          ,
                          log_name_file
#endif
    );

    assert(result->source == n(11));
    assert(result->target == n(111));
    assert(result->distance == 3);

    assert(array_list_ul_get(result->edges, 0) == 88);
    assert(array_list_ul_get(result->edges, 1) == 561);
    assert(array_list_ul_get(result->edges, 2) == 763);

    path_destroy(result);
    dict_ul_ul_destroy(map);
    dict_ul_d_destroy(heuristic);
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
#ifdef VERBOSE
    fclose(log_file);
#endif
}
