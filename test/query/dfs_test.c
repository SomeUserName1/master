#include "query/dfs.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "access/heap_file.h"
#include "query/result_types.h"

#define NUM_NODES (10)
static const unsigned long node0 = 0;
static const unsigned long node1 = 1;
static const unsigned long node2 = 2;
static const unsigned long node3 = 3;
static const unsigned long node4 = 4;
static const unsigned long node5 = 5;
static const unsigned long node6 = 6;
static const unsigned long node7 = 7;
static const unsigned long node8 = 8;
static const unsigned long node9 = 9;

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
    for (size_t i = 0; i < NUM_NODES; ++i) {
        create_node(hf, "\0");
    }

    create_relationship(hf, node0, node1, 1.0, "\0");
    create_relationship(hf, node0, node2, 1.0, "\0");
    create_relationship(hf, node0, node3, 1.0, "\0");

    create_relationship(hf, node1, node4, 1.0, "\0");
    create_relationship(hf, node1, node5, 1.0, "\0");

    create_relationship(hf, node2, node6, 1.0, "\0");
    create_relationship(hf, node2, node7, 1.0, "\0");

    create_relationship(hf, node3, node8, 1.0, "\0");
    create_relationship(hf, node3, node9, 1.0, "\0");

#ifdef VERBOSE
    const char* log_path = "/home/someusername/workspace_local/alt_test.txt";
    FILE*       log_file = fopen(log_path, "w+");

    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    traversal_result* result = dfs(hf,
                                   0,
                                   BOTH
#ifdef VERBOSE
                                   ,
                                   log_file
#endif
    );

    assert(result->source == 0);
    assert(dict_ul_ul_get_direct(result->traversal_numbers, 0) == 0);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 1) == 1);
    assert(dict_ul_ul_get_direct(result->parents, 1) == 0);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 2) == 1);
    assert(dict_ul_ul_get_direct(result->parents, 2) == 1);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 3) == 1);
    assert(dict_ul_ul_get_direct(result->parents, 3) == 2);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 8) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 8) == 7);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 9) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 9) == 8);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 6) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 6) == 5);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 7) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 7) == 6);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 4) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 4) == 3);

    assert(dict_ul_ul_get_direct(result->traversal_numbers, 5) == 2);
    assert(dict_ul_ul_get_direct(result->parents, 5) == 4);

    traversal_result_destroy(result);
    phy_database_delete(hf->cache->pdb);
    page_cache_destroy(hf->cache);
    heap_file_destroy(hf);
#ifdef VERBOSE
    fclose(log_file);
#endif
}
