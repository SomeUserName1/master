#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>

#include "constants.h"
#include "disk_file.h"

typedef enum
{
    node_file,
    relationship_file
} record_file;

typedef struct
{
    disk_file*    record_files[2];
    disk_file*    header_files[2];
    size_t        remaining_header_bits[2];
    unsigned char fst_header_cache[2][PAGE_SIZE];
} phy_database;

phy_database*
phy_database_create(char* db_name);

void
phy_database_delete(phy_database* db);

void
phy_database_close(phy_database* db);

void
phy_database_allocate_page(phy_database* db, record_file rf);

void
phy_database_deallocate_page(phy_database* db, record_file rf);

#endif
