#include "page.h"

#include <assert.h>

void
test_page_create(void)
{}

void
test_page_destroy(void)
{}

void
test_page_equals(void)
{}

void
test_read_ulong(void)
{}

void
test_write_ulong(void)
{}

void
test_read_uchar(void)
{}

void
test_write_uchar(void)
{}
void
test_read_double(void)
{}

void
test_write_double(void)
{}

void
test_read_string(void)
{}

void
test_write_string(void)
{}

void
test_page_pretty_print(void)
{}

int
main(void)
{
    test_page_create();
    test_page_destroy();
    test_page_equals();
    test_read_ulong();
    test_write_ulong();
    test_read_uchar();
    test_write_uchar();
    test_read_double();
    test_write_double();
    test_read_string();
    test_write_string();
    test_page_pretty_print();

    return 0;
}
