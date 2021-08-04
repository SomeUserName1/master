#include "physical_database.h"

#include <assert.h>

void
test_phy_database_create(void)
{}

void
test_phy_database_delete(void)
{}

void
test_phy_database_close(void)
{}

void
test_phy_database_validate_empty_header(void)
{}

void
test_phy_database_validate_header(void)
{}

void
test_allocate_pages(void)
{}

void
test_deallocate_pages(void)
{}

int
main(void)
{
    test_phy_database_create();
    test_phy_database_delete();
    test_phy_database_close();
    test_phy_database_validate_empty_header();
    test_phy_database_validate_header();
    test_allocate_pages();
    test_deallocate_pages();

    return 0;
}
