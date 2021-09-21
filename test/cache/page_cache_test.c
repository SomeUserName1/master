/*
 * @(#)page_cache_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "data-struct/htable.h"
#include "disk_file.h"
#include "page.h"
#include "page_cache.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/linked_list.h"
#include "physical_database.h"

void
test_page_cache_create(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );
    assert(pc);
    assert(pc->pdb == pdb);
    assert(pc->num_pins == 0);
    assert(pc->num_unpins == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES);

    assert(dict_ul_ul_size(pc->page_map[catalogue][0]) == 0);
    dict_ul_ul_destroy(pc->page_map[catalogue][0]);

    for (file_kind i = header; i < invalid; ++i) {
        for (file_type j = node_ft; j < invalid_ft; ++j) {
            printf("i %u, j %u\n", i, j);
            assert(dict_ul_ul_size(pc->page_map[i][j]) == 0);
            dict_ul_ul_destroy(pc->page_map[i][j]);
        }
    }

    assert(pc->cache);

#ifdef VERBOSE
    assert(pc->log_file);

    if (fclose(pc->log_file) != 0) {
        printf("page cache test - create: failed to close log file! %s\n",
               strerror(errno));
    }
#endif

    llist_ul_destroy(pc->free_frames);
    queue_ul_destroy(pc->recently_referenced);
    free(pc->cache[0]->data);
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        page_destroy(pc->cache[i]);
    }
    free(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - create successful!\n");
}

void
test_page_cache_destroy(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    page_cache_destroy(pc);

    assert(pdb);
    phy_database_delete(pdb);

    printf("Test Page Cache - destroy successful!\n");
}

void
test_new_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    page* n_page = new_page(pc, node_ft);

    assert(n_page);
    assert(n_page->page_no == pc->pdb->records[node_ft]->num_pages - 1);
    assert(n_page->dirty == false);
    assert(n_page->ft == node_ft);
    assert(n_page->fk == records);
    assert(n_page->pin_count == 1);

    n_page->pin_count = 0;
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - new page successful!\n");
}

void
test_pin_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, 3);

    page* test_page_1 = pin_page(pc, 0, records, node_ft);

    size_t frame_no_1 =
          dict_ul_ul_get_direct(pc->page_map[records][node_ft], 0);

    assert(pc->num_pins == 1);
    assert(pc->num_unpins == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 1);
    assert(queue_ul_size(pc->recently_referenced) == 0);

    assert(test_page_1);
    assert(pc->cache[frame_no_1] == test_page_1);
    assert(test_page_1->page_no == 0);
    assert(test_page_1->dirty == false);
    assert(test_page_1->ft == node_ft);
    assert(test_page_1->fk == records);
    assert(test_page_1->pin_count == 1);

    page* test_page_3 = pin_page(pc, 0, records, node_ft);
    assert(test_page_3);
    assert(test_page_3 == test_page_1);
    assert(test_page_3->pin_count == 2);
    assert(test_page_3->ft == node_ft);
    assert(test_page_3->fk == records);
    assert(pc->num_pins == 2);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 1);

    page* test_page_4 = pin_page(pc, 0, records, node_ft);
    assert(test_page_4);
    assert(test_page_4 == test_page_1);
    assert(test_page_4->pin_count == 3);
    assert(pc->num_pins == 3);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 1);
    test_page_1->pin_count = 0;
    queue_ul_append(pc->recently_referenced, 0);

    page*  test_page_2 = pin_page(pc, 2, records, node_ft);
    size_t frame_no_2 =
          dict_ul_ul_get_direct(pc->page_map[records][node_ft], 2);
    assert(test_page_2);
    assert(pc->cache[frame_no_2] == test_page_2);
    assert(test_page_2->page_no == 2);
    assert(test_page_2->dirty == false);
    assert(test_page_2->ft == node_ft);
    assert(test_page_2->fk == records);
    assert(test_page_2->pin_count == 1);
    assert(pc->num_pins == 4);

    test_page_2->pin_count = 0;
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - pin page successful!\n");
}

void
test_unpin_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, 1);

    page* test_page_1 = pin_page(pc, 0, records, node_ft);
    page* test_page_2 = pin_page(pc, 0, records, node_ft);

    size_t frame_no_1 =
          dict_ul_ul_get_direct(pc->page_map[records][node_ft], 0);

    assert(pc->num_pins == 2);
    assert(pc->num_unpins == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 1);
    assert(queue_ul_size(pc->recently_referenced) == 0);
    assert(test_page_1);
    assert(pc->cache[frame_no_1] == test_page_1);
    assert(test_page_1 == test_page_2);
    assert(test_page_1->page_no == 0);
    assert(test_page_1->dirty == false);
    assert(test_page_1->ft == node_ft);
    assert(test_page_1->fk == records);
    assert(test_page_1->pin_count == 2);

    unpin_page(pc, 0, records, node_ft);

    assert(test_page_1->pin_count == 1);
    assert(pc->num_unpins == 1);
    assert(pc->num_pins == 2);
    assert(queue_ul_size(pc->recently_referenced) == 1);
    assert(queue_ul_get(pc->recently_referenced, 0) == frame_no_1);

    unpin_page(pc, 0, records, node_ft);
    assert(pc->num_unpins == 2);
    assert(test_page_1->pin_count == 0);
    assert(queue_ul_size(pc->recently_referenced) == 1);
    assert(queue_ul_get(pc->recently_referenced, 0) == frame_no_1);

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - unpin page successful!\n");
}

void
test_evict(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, CACHE_N_PAGES + 1);

    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        pin_page(pc, i, records, node_ft);
    }

    assert(llist_ul_size(pc->free_frames) == 0);

    size_t frame_nos[EVICT_LRU_K];
    for (size_t i = 0; i < EVICT_LRU_K; ++i) {
        frame_nos[i] = dict_ul_ul_get_direct(pc->page_map[records][node_ft], i);
        unpin_page(pc, i, records, node_ft);
    }

    assert(llist_ul_size(pc->free_frames) == 0);

    size_t reads_before = pdb->records[node_ft]->read_count;
    pin_page(pc, 0, records, node_ft);
    assert(reads_before == pdb->records[node_ft]->read_count);
    assert(llist_ul_size(pc->free_frames) == 0);
    unpin_page(pc, 0, records, node_ft);

    evict(pc);

    assert(llist_ul_size(pc->free_frames) == EVICT_LRU_K);

    size_t max_evict =
          CACHE_N_PAGES < EVICT_LRU_K ? CACHE_N_PAGES : EVICT_LRU_K;
    for (size_t i = 0; i < max_evict; ++i) {
        assert(!dict_ul_ul_contains(pc->page_map[records][node_ft], i));
        // NOLINTNEXTLINE
        assert(!queue_ul_contains(pc->recently_referenced, frame_nos[i]));
    }

    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        pc->cache[i]->pin_count = 0;
        pc->cache[i]->dirty     = false;
    }

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - evict successful!\n");
}

void
test_flush_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, 1);

    page* p = pin_page(pc, 0, records, node_ft);
    write_ulong(p, 0, 1);
    unpin_page(pc, 0, records, node_ft);

    unsigned long writes_before = pdb->records[node_ft]->write_count;

    flush_page(pc, dict_ul_ul_get_direct(pc->page_map[records][node_ft], 0));

    assert(writes_before + 1 == pdb->records[node_ft]->write_count);

    unsigned char buf[PAGE_SIZE];
    read_page(pdb->records[node_ft], 0, buf);
    unsigned long content;
    memcpy(&content, buf, sizeof(unsigned long));

    assert(content == 1);
    assert(!p->dirty);

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - flush successful!\n");
}

void
test_flush_all_pages(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, CACHE_N_PAGES);

    page* p;
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        p = pin_page(pc, i, records, node_ft);
        write_ulong(p, 0, 1);
        pc->cache[i]->pin_count = 0;
    }

    flush_all_pages(pc);

    printf("finished flushing\n");

    unsigned char buf[PAGE_SIZE];
    unsigned long content;
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        read_page(pdb->records[node_ft], i, buf);
        content = 0;
        memcpy(&content, buf, sizeof(unsigned long));
        assert(content == 1);
    }

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - flush all successful!\n");
}

int
main(void)
{
    test_page_cache_create();
    test_page_cache_destroy();
    test_new_page();
    test_pin_page();
    test_unpin_page();
    test_evict();
    test_flush_page();
    test_flush_all_pages();

    return 0;
}
