#include "physical_database.h"

#include <errno.h>
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

    phy_db->files[node_header]         = disk_file_create(nodes_header_name);
    phy_db->files[relationship_header] = disk_file_create(rels_header_name);

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

    phy_db->files[node_file]         = disk_file_create(nodes_file_name);
    phy_db->files[relationship_file] = disk_file_create(rels_file_name);

    for (file_type i = 0; i <= relationship_header; ++i) {
        /* check if record file is empty. */

        /* If it is empty, write an empty page to the header file. Alternatively
         * check if the header is one page long and has zero entries. */
        /* The first 8 byte encode the number of used bits. */
        if (phy_db->files[i + 2]->file_size == 0) {

            phy_database_validate_empty_header(phy_db, i);
        } else {
            /* If the record file is not empty, the header is checked. */
            phy_database_validate_header(phy_db, i);
        }
    }

    return phy_db;
}

void
phy_database_validate_empty_header(phy_database* db, file_type i)
{

    /* first check if the header is also empty */
    if (db->files[i]->file_size != 0) {
        /* if not, validate that it has zero entries and is one page long. */

        unsigned char buf[sizeof(unsigned long)];
        unsigned long size;

        /* read the first 8 bytes */
        rewind(db->files[i]->file);
        if (fread(buf, sizeof(unsigned long), 1, db->files[i]->file)
            != sizeof(unsigned long)) {
            printf(" physical database - create: Failed to read the "
                   "header size from header file %s: %s\n",
                   db->files[i]->file_name,
                   strerror(errno));
            exit(EXIT_FAILURE);
        }
        /* convert them to an unsigned long */
        memcpy(&size, buf, sizeof(unsigned long));

        /* if it has more than zero entries or is larger than a page,
         * the header is invalid */
        if (size != 0 || db->files[i]->file_size != PAGE_SIZE) {
            printf("physical database - create: Non-empty header %s "
                   "for empty record file %s!\n",
                   db->files[i]->file_name,
                   db->files[i + 2]->file_name);
            exit(EXIT_FAILURE);
        }
    } else {
        /* if the header file is empty, write a page of zeros. */
        disk_file_grow(db->files[i], 1);
    }
    /* Mark all bits but the first 64 as unsused */
    db->remaining_header_bits[i] =
          (PAGE_SIZE - sizeof(unsigned long)) * CHAR_BIT;
}

void
phy_database_validate_header(phy_database* db, file_type i)
{
    unsigned char buf[sizeof(unsigned long)];
    unsigned long size;

    /* first check if the header is empty */
    if (db->files[i]->file_size == 0) {
        printf("physical database - create: Empty header %s for "
               "non-empty record file %s!\n",
               db->files[i]->file_name,
               db->files[i + 2]->file_name);
        exit(EXIT_FAILURE);
    }

    /* read the first 8 bytes */
    rewind(db->files[i]->file);
    if (fread(buf, sizeof(unsigned long), 1, db->files[i]->file)
        != sizeof(unsigned long)) {
        printf(" physical database - create: Failed to read the "
               "header size from header file %s: %s\n",
               db->files[i]->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* convert them to an unsigned long */
    memcpy(&size, buf, sizeof(unsigned long));

    /* caluclate the number of slots in the record file */
    unsigned char record_size =
          i + 2 == node_file ? NODE_RECORD_BYTES : RELATIONSHIP_RECORD_BYTES;

    size_t n_slots = (db->files[i + 2]->file_size / record_size);

    /* check if the number of slots and the size of the header match */
    if (size != n_slots) {
        printf(
              "physical database - validate header: Missmatch in the number of "
              "slots and entries between header %s: %zu and record %s: %zu!\n",
              db->files[i]->file_name,
              size,
              db->files[i + 2]->file_name,
              n_slots);
    }

    db->remaining_header_bits[i] =
          db->files[i]->file_size - (sizeof(unsigned long) * CHAR_BIT) - size;
}

void
phy_database_close(phy_database* db)
{
    if (!db) {
        printf("physical database - close: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    for (file_type i = 0; i <= relationship_file; ++i) {
        free(db->files[i]->file_name);
        disk_file_destroy(db->files[i]);
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

    for (file_type i = 0; i <= relationship_file; ++i) {
        disk_file_delete(db->files[i]);
        free(db->files[i]->file_name);
        free(db->files[i]);
    }

    free(db);
}

void
allocate_pages(phy_database* db, file_type rf, size_t num_pages)
{
    if (!db || rf <= relationship_header) {
        printf("physical database - allocate: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    disk_file_grow(db->files[rf], num_pages);

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

        disk_file_grow(db->files[rf - 2], additional_pages);

        db->remaining_header_bits[rf] +=
              (additional_pages * PAGE_SIZE * CHAR_BIT - additional_bits);
    } else {
        db->remaining_header_bits[rf] -= neccessary_bits;
    }

    /* Write the size back to disk */

    unsigned char file_bits[sizeof(unsigned long)];
    size_t        bits;

    rewind(db->files[rf - 2]->file);
    if (fread(file_bits, sizeof(unsigned long), 1, db->files[rf - 2]->file)
        != sizeof(unsigned long)) {
        printf(" physical database - allocate page: Failed to read the "
               "header size from header file %s: %s\n",
               db->files[rf - 2]->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    memcpy(&bits, file_bits, sizeof(unsigned long));

    bits += neccessary_bits;

    memcpy(file_bits, &bits, sizeof(unsigned long));
    rewind(db->files[rf - 2]->file);
    ;
    if (fwrite(file_bits, sizeof(unsigned long), 1, db->files[rf - 2]->file)
        != sizeof(unsigned long)) {
        printf(" physical database - allocate page: Failed to write the "
               "header size to header file %s: %s\n",
               db->files[rf - 2]->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void
deallocate_pages(phy_database* db, file_type rf)
{
    if (!db) {
        printf("physical database - deallocate page: Invalid "
               "arguments!\n");
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
