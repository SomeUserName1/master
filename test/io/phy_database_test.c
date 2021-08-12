#include "physical_database.h"

#include <assert.h>
#include <limits.h>

#include "constants.h"

void
test_phy_database_create(void)
{
    char* db_name = "test";

#ifdef VERBOSE
    char* log_file_name = "test_log";
#endif

    phy_database* pdb = phy_database_create(db_name
#ifdef VERBOSE
                                            ,
                                            log_file_name
#endif
    );

    assert(pdb);
    for (size_t i = 0; i < invalid; ++i) {
        assert(pdb->files[i]);
    }

    for (size_t i = 0; i < 2; ++i) {
        assert(pdb->remaining_header_bits[i]
               == (PAGE_SIZE - sizeof(unsigned long)) * CHAR_BIT);
    }
}

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
