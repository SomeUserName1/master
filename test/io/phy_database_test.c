#include "disk_file.h"
#include "physical_database.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

void
test_phy_database_validate_empty_header(void)
{
    phy_database* db = calloc(1, sizeof(*db));

    db->files[0] = disk_file_create("header_test");
    db->files[1] = disk_file_create("record_test");

    assert(phy_database_validate_empty_header(db, 0));

    unsigned char data[PAGE_SIZE];
    memset(data, 1, PAGE_SIZE);
    disk_file_grow(db->files[0], 1);
    write_page(db->files[0], 0, data);

    assert(!phy_database_validate_empty_header(db, 0));
    assert(db->remaining_header_bits[0]
           == (PAGE_SIZE - sizeof(unsigned long)) * CHAR_BIT);

    disk_file_delete(db->files[0]);
    disk_file_delete(db->files[1]);
    free(db);

    printf("test phy db validate empty header successfull!\n");
}

void
test_phy_database_validate_header(void)
{
    phy_database* db = calloc(1, sizeof(*db));

    db->files[0] = disk_file_create("header_test");
    db->files[1] = disk_file_create("record_test");

    unsigned char data[PAGE_SIZE];
    // Case 1: Empty header for non empty record file
    disk_file_grow(db->files[1], 1);
    memset(data, 1, PAGE_SIZE);
    write_page(db->files[1], 0, data);
    assert(!phy_database_validate_header(db, 0));

    // Case 2: smaller header than record file
    disk_file_grow(db->files[0], 1);
    memset(data, 0, PAGE_SIZE);
    data[0] = 1;
    write_page(db->files[0], 0, data);
    assert(!phy_database_validate_header(db, 0));

    // case 3: larger header than record file
    data[3] = UCHAR_MAX;
    write_page(db->files[0], 0, data);
    assert(!phy_database_validate_header(db, 0));

    // case 4: matching header and record file
    data[0] = 0;
    data[1] = 1;
    data[2] = 0;
    data[3] = 0;
    write_page(db->files[0], 0, data);

    assert(phy_database_validate_header(db, 0));

    unsigned long remaining_bits =
          (PAGE_SIZE - sizeof(unsigned long)) * CHAR_BIT
          - (PAGE_SIZE / SLOT_SIZE);

    assert(db->remaining_header_bits[0] == remaining_bits);

    disk_file_delete(db->files[0]);
    disk_file_delete(db->files[1]);
    free(db);

    printf("test phy db validate header successfull!\n");
}

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

    phy_database_delete(pdb);
    printf("test phy db create successfull!\n");
}

void
test_phy_database_delete(void)
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

    phy_database_delete(pdb);

    assert(!fopen("test_nodes.db", "r"));
    assert(!fopen("test_relationships.db", "r"));
    assert(!fopen("test_nodes.idx", "r"));
    assert(!fopen("test_relationships.idx", "r"));

#ifdef VERBOSE
    FILE* log = fopen("test_log", "r");
    assert(log);
    fclose("test_log");
#endif
}

void
test_phy_database_close(void)
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

    phy_database_close(pdb);

    FILE* nodes   = fopen("test_nodes.db", "r");
    FILE* rels    = fopen("test_relationships.db", "r");
    FILE* nheader = fopen("test_nodes.idx", "r");
    FILE* rheader = fopen("test_relationships.idx", "r");

    assert(nodes);
    assert(rels);
    assert(nheader);
    assert(rheader);

    remove("test_nodes.db");
    remove("test_relationships.db");
    remove("test_nodes.idx");
    remove("test_relationships.idx");
}

void
test_allocate_pages(void)
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

    allocate_pages(pdb, node_file, 1);

    assert(pdb->files[node_file]->num_pages == 1);
    assert(pdb->files[node_file]->file_size == PAGE_SIZE);
    assert(pdb->files[node_header]->num_pages == 1);
    assert(pdb->files[node_header]->file_size == PAGE_SIZE);
    assert(pdb->remaining_header_bits[node_header]
           == (PAGE_SIZE - sizeof(unsigned long)) * CHAR_BIT
                    - PAGE_SIZE / SLOT_SIZE);

    // PAGE_SIZE is here just to test sth. larger than 1
    allocate_pages(pdb, node_file, PAGE_SIZE - 1);

    assert(pdb->files[node_file]->num_pages == PAGE_SIZE);
    assert(pdb->files[node_file]->file_size == PAGE_SIZE * PAGE_SIZE);
    // Page size many pages * (256 bit for each page / 8 to get byte / Page size
    // to get pages )
    assert(pdb->files[node_header]->num_pages
           == 1 + (PAGE_SIZE / SLOT_SIZE) / 8);

    assert(pdb->files[node_header]->file_size
           == PAGE_SIZE * (1 + (PAGE_SIZE / SLOT_SIZE) / 8));

    unsigned long remaining_bits =
          (PAGE_SIZE * (1 + (PAGE_SIZE / SLOT_SIZE) / CHAR_BIT)
           - sizeof(unsigned long))
                * CHAR_BIT
          - PAGE_SIZE * (PAGE_SIZE / SLOT_SIZE);

    printf("real %lu check %lu\n",
           pdb->remaining_header_bits[node_header],
           remaining_bits);
    assert(pdb->remaining_header_bits[node_header] == remaining_bits);

    phy_database_delete(pdb);
}

void
test_phy_database_open(void)
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

    allocate_pages(pdb, node_file, PAGE_SIZE);
    unsigned char data[PAGE_SIZE];
    memset(data, 1, PAGE_SIZE);
    write_page(pdb->files[node_file], 0, data);

    phy_database_close(pdb);

    memset(data, 0, PAGE_SIZE);

    pdb = phy_database_open(db_name
#ifdef VERBOSE
                            ,
                            log_file_name
#endif
    );

    read_page(pdb->files[node_file], 0, data);

    for (size_t i = 0; i < PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    phy_database_delete(pdb);
}

void
test_deallocate_pages(void)
{
    printf("test deallocate pages: TODO");
}

void
test_defragment(void)
{
    printf("test defragment: TODO");
}

int
main(void)
{
    test_phy_database_validate_empty_header();
    test_phy_database_validate_header();
    test_phy_database_create();
    test_phy_database_delete();
    test_phy_database_close();
    test_allocate_pages();
    test_phy_database_open();
    test_deallocate_pages();
    test_defragment();

    return 0;
}
