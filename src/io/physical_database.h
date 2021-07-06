#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>

#include "constants.h"
#include "disk_file.h"

typedef enum
{
    node_header,
    relationship_header,
    node_file,
    relationship_file
} file_type;

typedef struct
{
    disk_file* files[4];
    size_t     remaining_header_bits[2];
} phy_database;

phy_database*
phy_database_create(char* db_name);

void
phy_database_delete(phy_database* db);

void
phy_database_close(phy_database* db);

void
phy_database_validate_empty_header(phy_database* db, file_type i);

void
phy_database_validate_header(phy_database* db, file_type i);

void
phy_database_allocate_page(phy_database* db, file_type rf);

void
phy_database_deallocate_page(phy_database* db, file_type rf);

#endif
