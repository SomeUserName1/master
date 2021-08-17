#include "page_cache.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
    assert(pc->cache);
    // TODO continue here
}

void
test_page_cache_destroy(void)
{}

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
