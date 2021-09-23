/*
 * @(#)phy_database_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "disk_file.h"
#include "physical_database.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

void
test_phy_database_validate_empty_header(void)
{
    phy_database* db = calloc(1, sizeof(*db));

    FILE* log_file = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    db->catalogue  = disk_file_create("catalogue_test", log_file);
    db->header[0]  = disk_file_create("header_test", log_file);
    db->records[0] = disk_file_create("record_test", log_file);

    assert(phy_database_validate_empty_header(db, 0));

    unsigned char data[PAGE_SIZE];
    memset(data, 1, PAGE_SIZE);
    disk_file_grow(db->catalogue, 1, false);
    write_page(db->catalogue, 0, data, false);

    assert(!phy_database_validate_empty_header(db, 0));
    assert(db->remaining_header_bits[0] == PAGE_SIZE * CHAR_BIT);

    disk_file_delete(db->catalogue);
    disk_file_delete(db->header[0]);
    disk_file_delete(db->records[0]);
    free(db);

    printf("test phy db validate empty header successfull!\n");
}

void
test_phy_database_validate_header(void)
{
    phy_database* db = calloc(1, sizeof(*db));

    FILE* log_file = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    db->catalogue = disk_file_create("catalogue_test", log_file);
    disk_file_grow(db->catalogue, 1, false);
    db->header[0]  = disk_file_create("header_test", log_file);
    db->records[0] = disk_file_create("record_test", log_file);

    unsigned char data[PAGE_SIZE];
    // Case 1: Empty header for non empty record file
    disk_file_grow(db->records[0], 1, false);
    memset(data, 1, PAGE_SIZE);
    write_page(db->records[0], 0, data, false);
    assert(!phy_database_validate_header(db, 0));

    // Case 2: smaller header than record file
    disk_file_grow(db->header[0], 1, false);
    memset(data, 0, PAGE_SIZE);
    data[0] = 1;
    write_page(db->catalogue, 0, data, false);
    assert(!phy_database_validate_header(db, 0));

    // case 3: larger header than record file
    data[3] = UCHAR_MAX;
    write_page(db->catalogue, 0, data, false);
    assert(!phy_database_validate_header(db, 0));

    // case 4: matching header and record file
    data[0] = 0;
    data[1] = 1;
    data[2] = 0;
    data[3] = 0;
    write_page(db->catalogue, 0, data, false);

    assert(phy_database_validate_header(db, 0));

    unsigned long remaining_bits =
          PAGE_SIZE * CHAR_BIT - (PAGE_SIZE / SLOT_SIZE);

    assert(db->remaining_header_bits[0] == remaining_bits);

    disk_file_delete(db->catalogue);
    disk_file_delete(db->header[0]);
    disk_file_delete(db->records[0]);
    free(db);

    printf("test phy db validate header successfull!\n");
}

void
test_phy_database_create(void)
{
    char* db_name = "test";

    char* log_file_name = "test_log";

    phy_database* pdb = phy_database_create(db_name, log_file_name);

    assert(pdb);
    assert(pdb->catalogue);
    for (file_type i = 0; i < invalid_ft; ++i) {
        assert(pdb->header[i]);
        assert(pdb->records[i]);
        assert(pdb->remaining_header_bits[i] == PAGE_SIZE * CHAR_BIT);
    }

    phy_database_delete(pdb);
    printf("test phy db create successfull!\n");
}

void
test_phy_database_delete(void)
{
    char* db_name = "test";

    char* log_file_name = "test_log";

    phy_database* pdb = phy_database_create(db_name, log_file_name);

    assert(pdb);

    phy_database_delete(pdb);

    assert(!fopen("test.info", "r"));
    assert(!fopen("test_nodes.db", "r"));
    assert(!fopen("test_relationships.db", "r"));
    assert(!fopen("test_nodes.idx", "r"));
    assert(!fopen("test_relationships.idx", "r"));

    printf("test phy db delete successfull!\n");
}

void
test_phy_database_close(void)
{
    char* db_name = "test";

    char* log_file_name = "test_log";

    phy_database* pdb = phy_database_create(db_name, log_file_name);

    assert(pdb);

    phy_database_close(pdb);

    FILE* catalogue = fopen("test.info", "r");
    FILE* nodes     = fopen("test_nodes.db", "r");
    FILE* rels      = fopen("test_relationships.db", "r");
    FILE* nheader   = fopen("test_nodes.idx", "r");
    FILE* rheader   = fopen("test_relationships.idx", "r");

    assert(catalogue);
    assert(nodes);
    assert(rels);
    assert(nheader);
    assert(rheader);

    remove("test.info");
    remove("test_nodes.db");
    remove("test_relationships.db");
    remove("test_nodes.idx");
    remove("test_relationships.idx");

    printf("test phy db close successfull!\n");
}

void
test_allocate_pages(void)
{
    char* db_name = "test";

    char* log_file_name = "test_log";

    phy_database* pdb = phy_database_create(db_name, log_file_name);

    allocate_pages(pdb, node_ft, 1, true);

    assert(pdb->records[node_ft]->num_pages == 1);
    assert(pdb->records[node_ft]->file_size == PAGE_SIZE);
    assert(pdb->header[node_ft]->num_pages == 1);
    assert(pdb->header[node_ft]->file_size == PAGE_SIZE);
    assert(pdb->remaining_header_bits[node_ft]
           == PAGE_SIZE * CHAR_BIT - PAGE_SIZE / SLOT_SIZE);

    // PAGE_SIZE is here just to test sth. larger than 1
    allocate_pages(pdb, node_ft, PAGE_SIZE - 1, false);

    assert(pdb->records[node_ft]->num_pages == PAGE_SIZE);
    assert(pdb->records[node_ft]->file_size == PAGE_SIZE * PAGE_SIZE);
    // Page size many pages * (256 bit for each page / 8 to get byte / Page size
    // to get pages )
    assert(pdb->header[node_ft]->num_pages == (PAGE_SIZE / SLOT_SIZE) / 8);

    assert(pdb->header[node_ft]->file_size
           == PAGE_SIZE * (PAGE_SIZE / SLOT_SIZE) / 8);

    unsigned long remaining_bits =
          (PAGE_SIZE * (PAGE_SIZE / SLOT_SIZE) / CHAR_BIT) * CHAR_BIT
          - PAGE_SIZE * (PAGE_SIZE / SLOT_SIZE);

    assert(pdb->remaining_header_bits[node_ft] == remaining_bits);

    phy_database_delete(pdb);

    printf("test phy db allocate pages successfull!\n");
}

void
test_phy_database_open(void)
{
    char* db_name = "test";

    char* log_file_name = "test_log";

    phy_database* pdb = phy_database_create(db_name, log_file_name);

    allocate_pages(pdb, node_ft, PAGE_SIZE, false);
    unsigned char data[PAGE_SIZE];
    memset(data, 1, PAGE_SIZE);
    write_page(pdb->records[node_ft], 0, data, false);

    phy_database_close(pdb);

    memset(data, 0, PAGE_SIZE);

    pdb = phy_database_open(db_name, log_file_name);

    read_page(pdb->records[node_ft], 0, data, false);

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
