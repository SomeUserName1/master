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

static const char* log_path =
      "/home/someusername/workspace_local/degree_test.txt";
static FILE* log_file;

heap_file*
prepare(void)
{

    log_file = fopen(log_path, "w+");
    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char* file_name = "test";

    char* log_name_phf   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";

    phy_database* phf = phy_database_create(file_name

                                            ,
                                            log_name_phf

    );

    allocate_pages(phf, node_ft, 1, false);
    allocate_pages(phf, relationship_ft, 1, false);

    page_cache* pc = page_cache_create(phf,
                                       CACHE_N_PAGES

                                       ,
                                       log_name_cache

    );

    heap_file* hf = heap_file_create(pc

                                     ,
                                     log_name_file

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

    fclose(log_file);
}

void
test_get_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);

    create_relationship(hf, 0, 1, 1.0, "\0", false);
    create_relationship(hf, 0, 2, 1.0, "\0", false);
    create_relationship(hf, 0, 3, 1.0, "\0", false);
    create_relationship(hf, 1, 0, 1.0, "\0", false);
    printf("t1\n");

    assert(get_degree(hf, 0, BOTH, true, log_file

                      )
           == 4);
    printf("t2\n");
    assert(get_degree(hf, 0, OUTGOING, false, log_file

                      )
           == 3);
    printf("t3\n");
    assert(get_degree(hf, 0, INCOMING, false, log_file

                      )
           == 1);
    printf("t4\n");

    clean_up(hf);
}

void
test_get_avg_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);

    create_relationship(hf, 0, 1, 1.0, "\0", false);
    create_relationship(hf, 0, 2, 1.0, "\0", false);
    create_relationship(hf, 0, 3, 1.0, "\0", false);
    create_relationship(hf, 1, 0, 1.0, "\0", false);
    create_relationship(hf, 2, 3, 1.0, "\0", false);

    float avg_deg = get_avg_degree(hf, BOTH, true, log_file

    );
    assert(avg_deg > TRUE_AVG_DEG_BOTH_LOW && avg_deg < TRUE_AVG_DEG_BOTH_HIGH);

    avg_deg = get_avg_degree(hf, OUTGOING, false, log_file

    );
    assert(avg_deg > TRUE_AVG_DEG_OUT_LOW && avg_deg < TRUE_AVG_DEG_OUT_HIGH);

    avg_deg = get_avg_degree(hf, INCOMING, false, log_file

    );
    assert(avg_deg > TRUE_AVG_DEG_INC_LOW && avg_deg < TRUE_AVG_DEG_INC_HIGH);

    assert(get_avg_degree(hf, OUTGOING, false, log_file

                          )
           == get_avg_degree(hf, INCOMING, false, log_file

                             ));

    clean_up(hf);
}

void
test_get_min_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "\0", false);
    create_node(hf, "\0", false);
    create_node(hf, "\0", false);
    create_node(hf, "\0", false);

    create_relationship(hf, 0, 1, 1.0, "\0", false);
    create_relationship(hf, 0, 2, 1.0, "\0", false);
    create_relationship(hf, 0, 3, 1.0, "\0", false);
    create_relationship(hf, 1, 0, 1.0, "\0", false);
    create_relationship(hf, 2, 3, 1.0, "\0", false);

    assert(get_min_degree(hf, OUTGOING, true, log_file

                          )
           == 0);
    assert(get_min_degree(hf, BOTH, false, log_file

                          )
           == 2);
    assert(get_min_degree(hf, INCOMING, false, log_file

                          )
           == 1);

    clean_up(hf);
}

void
text_get_max_degree(void)
{
    heap_file* hf = prepare();

    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);
    create_node(hf, "in_memory_create_node(hf)", false);

    create_relationship(hf, 0, 1, 1.0, "\0", false);
    create_relationship(hf, 0, 2, 1.0, "\0", false);
    create_relationship(hf, 0, 3, 1.0, "\0", false);
    create_relationship(hf, 1, 0, 1.0, "\0", false);
    create_relationship(hf, 2, 3, 1.0, "\0", false);

    assert(get_max_degree(hf, OUTGOING, true, log_file

                          )
           == 3);
    assert(get_max_degree(hf, BOTH, false, log_file

                          )
           == 4);
    assert(get_max_degree(hf, INCOMING, false, log_file

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
