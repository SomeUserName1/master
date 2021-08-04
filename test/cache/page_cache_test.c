#include "page_cache.h"

void
test_page_cache_create(void);

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
