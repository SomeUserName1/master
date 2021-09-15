/*
 * @(#)degree_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "physical_database.h"
#include "query/degree.h"

#include "access/heap_file.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

static const float TRUE_AVG_DEG_BOTH_LOW  = 2.4F;
static const float TRUE_AVG_DEG_BOTH_HIGH = 2.6F;

static const float TRUE_AVG_DEG_OUT_LOW  = 1.1F;
static const float TRUE_AVG_DEG_OUT_HIGH = 1.3F;

static const float TRUE_AVG_DEG_INC_LOW  = 1.1F;
static const float TRUE_AVG_DEG_INC_HIGH = 1.3F;

#ifdef VERBOSE
static const char* log_path = "/home/someusername/workspace_local/alt_test.txt";
static FILE*       log_file;
#endif

heap_file*
prepare(void)
{
#ifdef VERBOSE
    log_file = fopen(log_path, "w+");
    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_phf   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* phf = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_phf
#endif
    );

    allocate_pages(phf, node_ft, 1);
    allocate_pages(phf, relationship_ft, 1);

    page_cache* pc = page_cache_create(phf
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
    return hf;
}

void
clean_up(heap_file* hf)
{
    page_cache*   pc  = hf->cache;
    phy_database* pdb = pc->pdb;
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

#ifdef VERBOSE
    fclose(log_file);
#endif
}

void
test_get_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");

    create_relationship(hf, 0, 1, 1.0, "\0");
    create_relationship(hf, 0, 2, 1.0, "\0");
    create_relationship(hf, 0, 3, 1.0, "\0");
    create_relationship(hf, 1, 0, 1.0, "\0");
    printf("t1\n");

    assert(get_degree(hf,
                      0,
                      BOTH
#ifdef VERBOSE
                      ,
                      log_file
#endif
                      )
           == 4);
    printf("t2\n");
    assert(get_degree(hf,
                      0,
                      OUTGOING
#ifdef VERBOSE
                      ,
                      log_file
#endif
                      )
           == 3);
    printf("t3\n");
    assert(get_degree(hf,
                      0,
                      INCOMING
#ifdef VERBOSE
                      ,
                      log_file
#endif
                      )
           == 1);
    printf("t4\n");

    clean_up(hf);
}

void
test_get_avg_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");

    create_relationship(hf, 0, 1, 1.0, "\0");
    create_relationship(hf, 0, 2, 1.0, "\0");
    create_relationship(hf, 0, 3, 1.0, "\0");
    create_relationship(hf, 1, 0, 1.0, "\0");
    create_relationship(hf, 2, 3, 1.0, "\0");

    float avg_deg = get_avg_degree(hf,
                                   BOTH
#ifdef VERBOSE
                                   ,
                                   log_file
#endif
    );
    assert(avg_deg > TRUE_AVG_DEG_BOTH_LOW && avg_deg < TRUE_AVG_DEG_BOTH_HIGH);

    avg_deg = get_avg_degree(hf,
                             OUTGOING
#ifdef VERBOSE
                             ,
                             log_file
#endif
    );
    assert(avg_deg > TRUE_AVG_DEG_OUT_LOW && avg_deg < TRUE_AVG_DEG_OUT_HIGH);

    avg_deg = get_avg_degree(hf,
                             INCOMING
#ifdef VERBOSE
                             ,
                             log_file
#endif
    );
    assert(avg_deg > TRUE_AVG_DEG_INC_LOW && avg_deg < TRUE_AVG_DEG_INC_HIGH);

    assert(get_avg_degree(hf,
                          OUTGOING
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == get_avg_degree(hf,
                             INCOMING
#ifdef VERBOSE
                             ,
                             log_file
#endif
                             ));

    clean_up(hf);
}

void
test_get_min_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "\0");
    create_node(hf, "\0");
    create_node(hf, "\0");
    create_node(hf, "\0");

    create_relationship(hf, 0, 1, 1.0, "\0");
    create_relationship(hf, 0, 2, 1.0, "\0");
    create_relationship(hf, 0, 3, 1.0, "\0");
    create_relationship(hf, 1, 0, 1.0, "\0");
    create_relationship(hf, 2, 3, 1.0, "\0");

    assert(get_min_degree(hf,
                          OUTGOING
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == 0);
    assert(get_min_degree(hf,
                          BOTH
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == 2);
    assert(get_min_degree(hf,
                          INCOMING
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == 1);

    clean_up(hf);
}

void
text_get_max_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");
    create_node(hf, "in_memory_create_node(hf)");

    create_relationship(hf, 0, 1, 1.0, "\0");
    create_relationship(hf, 0, 2, 1.0, "\0");
    create_relationship(hf, 0, 3, 1.0, "\0");
    create_relationship(hf, 1, 0, 1.0, "\0");
    create_relationship(hf, 2, 3, 1.0, "\0");

    assert(get_max_degree(hf,
                          OUTGOING
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == 3);
    assert(get_max_degree(hf,
                          BOTH
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == 4);
    assert(get_max_degree(hf,
                          INCOMING
#ifdef VERBOSE
                          ,
                          log_file
#endif
                          )
           == 2);

    clean_up(hf);
}

int
main(void)
{
    test_get_degree();
    printf("test1\n");
    test_get_avg_degree();
    printf("test2\n");
    test_get_min_degree();
    printf("test3\n");
    text_get_max_degree();
    printf("test4\n");

    printf("Tested degree functions successfully!\n");
}
