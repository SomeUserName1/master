#include "data-struct/bitmap.h"
#include "data-struct/htable.h"
#include "page_cache.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );
    assert(pc);
    assert(pc->pdb == pdb);
    assert(pc->total_pinned == 0);
    assert(pc->total_unpinned == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES);
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        assert(get_bit(pc->pinned, i) == 0);
    }
    assert(queue_ul_size(pc->recently_referenced) == 0);

    for (size_t i = 0; i < invalid; ++i) {
        assert(dict_ul_ul_size(pc->page_map[i]) == 0);
        dict_ul_ul_destroy(pc->page_map[i]);
    }

    assert(pc->cache);
    bitmap_destroy(pc->pinned);
    llist_ul_destroy(pc->free_frames);
    queue_ul_destroy(pc->recently_referenced);
    free(pc);
    phy_database_delete(pdb);
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
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    page_cache_destroy(pc);

    assert(pdb);
    phy_database_delete(pdb);
}

void
test_new_page(void)
{}

void
test_pin_page(void)
{}

void
test_unpin_page(void)
{}

void
test_evict_page(void)
{}

void
test_flush_page(void)
{}

void
test_flush_all_pages(void)
{}

int
main(void)
{
    test_page_cache_create();
    test_page_cache_destroy();
    test_new_page();
    test_pin_page();
    test_unpin_page();
    test_evict_page();
    test_flush_page();
    test_flush_all_pages();

    return 0;
}
