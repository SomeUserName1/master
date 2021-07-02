#include "physical_database.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "disk_file.h"

#define NODE_FILE_POSTFIX_LEN (strlen("_nodes.db"))
#define RELS_FILE_POSTFIX_LEN (strlen("_relationships.db"))

#define NODE_HEADER_POSTFIX_LEN (strlen("_nodes.idx"))
#define RELS_HEADER_POSTFIX_LEN (strlen("_relationships.idx"))

// TODO FWRITE ERROR CHECKING

phy_database*
phy_database_create(char* db_name)
{
    if (!db_name) {
        printf("physical database - create: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    phy_database* phy_db = calloc(1, sizeof(phy_database));

    /* Create or open header files for the record files */
    // +1 as strlen does not count \0
    char* nodes_header_name =
          calloc(strlen(db_name) + NODE_HEADER_POSTFIX_LEN + 1, sizeof(char));
    char* rels_header_name =
          calloc(strlen(db_name) + RELS_HEADER_POSTFIX_LEN + 1, sizeof(char));

    strncpy(nodes_header_name, db_name, strlen(db_name) + 1);
    strncpy(rels_header_name, db_name, strlen(db_name) + 1);

    strncat(nodes_header_name, "_nodes.idx", NODE_HEADER_POSTFIX_LEN);
    strncat(rels_header_name, "_relationships.idx", RELS_HEADER_POSTFIX_LEN);

    phy_db->header_files[node_file] = disk_file_create(nodes_header_name);
    phy_db->header_files[relationship_file] =
          disk_file_create(rels_header_name);

    /* Create or open Record files */
    // +1 as strlen does not count \0
    char* nodes_file_name =
          calloc(strlen(db_name) + NODE_FILE_POSTFIX_LEN + 1, sizeof(char));
    char* rels_file_name =
          calloc(strlen(db_name) + RELS_FILE_POSTFIX_LEN + 1, sizeof(char));

    strncpy(nodes_file_name, db_name, strlen(db_name) + 1);
    strncpy(rels_file_name, db_name, strlen(db_name) + 1);

    strncat(nodes_file_name, "_nodes.db", NODE_FILE_POSTFIX_LEN);
    strncat(rels_file_name, "_relationships.db", RELS_FILE_POSTFIX_LEN);

    phy_db->record_files[node_file]         = disk_file_create(nodes_file_name);
    phy_db->record_files[relationship_file] = disk_file_create(rels_file_name);

    unsigned char init_bits[sizeof(unsigned long)] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for (size_t i = 0; i < 2; ++i) {
        phy_db->remaining_header_bits[i] =
              (PAGE_SIZE - sizeof(unsigned long)) * CHAR_BIT;
        fwrite(init_bits,
               sizeof(unsigned long),
               1,
               phy_db->header_files[i]->file);
    }

    return phy_db;
}

void
phy_database_close(phy_database* db)
{
    if (!db) {
        printf("physical database - close: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < 2; ++i) {
        free(db->record_files[i]->file_name);
        free(db->header_files[i]->file_name);

        disk_file_destroy(db->record_files[i]);
        disk_file_destroy(db->header_files[i]);
    }

    free(db);
}

void
phy_database_delete(phy_database* db)
{
    if (!db) {
        printf("physical database - delete: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < 2; ++i) {
        disk_file_delete(db->record_files[i]);
        disk_file_delete(db->header_files[i]);

        free(db->record_files[i]->file_name);
        free(db->header_files[i]->file_name);

        free(db->record_files[i]);
        free(db->header_files[i]);
    }

    free(db);
}

void
allocate_pages(phy_database* db, record_file rf, size_t num_pages)
{
    if (!db) {
        printf("physical database - allocate: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    disk_file_grow(db->record_files[rf], num_pages);

    unsigned char record_size =
          rf == node_file ? NODE_RECORD_BYTES : RELATIONSHIP_RECORD_BYTES;
    size_t neccessary_bits = num_pages * (PAGE_SIZE / record_size);

    if (db->remaining_header_bits[rf] < neccessary_bits) {
        size_t additional_bits =
              neccessary_bits - db->remaining_header_bits[rf];
        size_t additional_bytes =
              (additional_bits / CHAR_BIT) + (additional_bits % CHAR_BIT != 0);
        size_t additional_pages = (additional_bytes / PAGE_SIZE)
                                  + (additional_bytes % PAGE_SIZE != 0);

        disk_file_grow(db->header_files[rf], additional_pages);
        /* Write the size back to disk */
        rewind(db->header_files[rf]->file);
        unsigned char file_bits[sizeof(unsigned long)];
        fread(file_bits, sizeof(unsigned long), 1, db->header_files[rf]->file);
        size_t bits;
        memcpy(&bits, file_bits, sizeof(unsigned long));
        bits += neccessary_bits;
        memcpy(file_bits, &bits, sizeof(unsigned long));
        rewind(db->header_files[rf]->file);
        fwrite(file_bits, sizeof(unsigned long), 1, db->header_files[rf]->file);

        db->remaining_header_bits[rf] =
              additional_pages * PAGE_SIZE * CHAR_BIT - additional_bits;
    }
}

void
deallocate_pages(phy_database* db, record_file rf)
{
    if (!db) {
        printf("physical database - deallocate page: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    printf("pyhsical database - dealloc: Not Implemented! %p, %i\n", db, rf);
    exit(-1);
}

void
physical_database_defragment(phy_database* pdb)
{
    printf("pyhsical database - defragment: Not Implemented! %p\n", pdb);
    exit(-1);
}
