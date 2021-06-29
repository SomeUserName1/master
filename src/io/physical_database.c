#include "physical_database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk_file.h"

#define NODE_FILE_POSTFIX_LEN 9
#define RELS_FILE_POSTFIX_LEN 17

phy_database*
phy_database_create(char* db_name)
{
    if (!db_name) {
        printf("physical database - create: Invalid arguments!\n");
        exit(-1);
    }

    phy_database* phy_db = calloc(1, sizeof(phy_database));

    // +1 as strlen does not count \0
    char* nodes_file_name =
          calloc(strlen(db_name) + NODE_FILE_POSTFIX_LEN + 1, sizeof(char));
    char* rels_file_name =
          calloc(strlen(db_name) + RELS_FILE_POSTFIX_LEN + 1, sizeof(char));

    strncpy(nodes_file_name, db_name, strlen(db_name) + 1);
    strncpy(rels_file_name, db_name, strlen(db_name) + 1);

    strncat(nodes_file_name, "_nodes.db", NODE_FILE_POSTFIX_LEN);
    strncat(rels_file_name, "_relationships.db", RELS_FILE_POSTFIX_LEN);

    phy_db->node_file   = disk_file_create(nodes_file_name);
    phy_db->rel_file    = disk_file_create(rels_file_name);
    phy_db->read_count  = 0;
    phy_db->write_count = 0;

    return phy_db;
}

void
phy_database_close(phy_database* db)
{
    if (!db) {
        printf("physical database - create: Invalid arguments!\n");
        exit(-1);
    }

    free(db->node_file->file_name);
    free(db->rel_file->file_name);

    disk_file_destroy(db->node_file);
    disk_file_destroy(db->rel_file);

    free(db);
}

void
phy_database_delete(phy_database* db)
{
    if (!db) {
        printf("physical database - create: Invalid arguments!\n");
        exit(-1);
    }

    disk_file_delete(db->node_file);
    disk_file_delete(db->rel_file);

    free(db->node_file->file_name);
    free(db->rel_file->file_name);

    free(db->node_file);
    free(db->rel_file);
    free(db);
}
