#include "disk_file.h"

#include <assert.h>

void
test_disk_file_create(void)
{}

void
test_disk_file_destroy(void)
{}

void
test_disk_file_delete(void)
{}

void
test_read_page(void)
{}

void
test_read_pages(void)
{}

void
test_write_page(void)
{}

void
test_write_pages(void)
{}

void
test_clear_page(void)
{}

void
test_disk_file_grow(void)
{}

void
test_disk_file_shrink(void)
{}

int
main(void)
{
    test_disk_file_create();
    test_disk_file_destroy();
    test_disk_file_delete();
    test_read_page();
    test_read_pages();
    test_write_page();
    test_write_pages();
    test_clear_page();
    test_disk_file_grow();
    test_disk_file_shrink();

    return 0;
}
