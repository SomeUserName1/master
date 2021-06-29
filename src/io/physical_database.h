#ifndef DATABASE_H
#define DATABASE_H

#include "disk_file.h"

#include <stddef.h>

typedef struct
{
    disk_file* node_file;
    disk_file* rel_file;
    size_t     read_count;
    size_t     write_count;
} phy_database;

phy_database*
phy_database_create(char* db_name);

void
phy_database_delete(phy_database* db);

void
phy_database_close(phy_database* db);

#endif
