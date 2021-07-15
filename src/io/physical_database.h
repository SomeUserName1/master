#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>

#include "disk_file.h"

typedef enum
{
#ifdef ADJ_LIST
    header_file,
    record_file
#else
    node_header,
    node_file,
    relationship_header,
    relationship_file,
#endif
          invalid
} file_type;

typedef struct
{
#ifdef ADJ_LIST
    disk_file* files[2];
    size_t     remaining_header_bits[1];
#else
    disk_file* files[4];
    size_t remaining_header_bits[2];
#endif
} phy_database;

phy_database*
phy_database_create(char* db_name);

void
phy_database_delete(phy_database* db);

void
phy_database_close(phy_database* db);

void
phy_database_validate_empty_header(phy_database* db, file_type ft);

void
phy_database_validate_header(phy_database* db, file_type ft);

void
phy_database_allocate_page(phy_database* db, file_type ft);

void
phy_database_deallocate_page(phy_database* db, file_type ft);

#endif
