/*
 * @(#)physical_database.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <stddef.h>

#include "disk_file.h"

typedef enum
{
#ifdef ADJ_LIST
    singleton,
#else
    node_ft,
    relationship_ft,
#endif
    invalid_ft
} file_type;

// file type and kind are used informally here.
// No functional programming theory black magic
typedef enum
{
    catalogue,
    header,
    records,
    invalid
} file_kind;

typedef struct
{
    disk_file* catalogue;
#ifdef ADJ_LIST
    disk_file* header[1];
    disk_file* records[1];
    size_t     remaining_header_bits[1];
#else
    disk_file* header[2];
    disk_file* records[2];
    size_t remaining_header_bits[2];
#endif
#ifdef VERBOSE
    FILE* log_file;
#endif
} phy_database;

phy_database*
phy_database_create(char* db_name
#ifdef VERBOSE
                    ,
                    const char* log_file
#endif
);

phy_database*
phy_database_open(char* db_name
#ifdef VERBOSE
                  ,
                  const char* log_file
#endif
);

void
phy_database_delete(phy_database* db);

void
phy_database_close(phy_database* db);

bool
phy_database_validate_empty_header(phy_database* db, file_type ft);

bool
phy_database_validate_header(phy_database* db, file_type ft);

void
allocate_pages(phy_database* db, file_type ft, size_t num_pages);

void
deallocate_pages(phy_database* db, file_type ft);

void
physical_database_defragment(phy_database* pdb);
#endif
