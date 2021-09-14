/*
 * @(#)physical_database.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "physical_database.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "disk_file.h"

#define CATALOGUE_POSTFIX_LEN (strlen(".info"))

#define NODE_FILE_POSTFIX_LEN   (strlen("_nodes.db"))
#define RELS_FILE_POSTFIX_LEN   (strlen("_relationships.db"))
#define RECORD_FILE_POSTFIX_LEN (strlen(".db"))

#define NODE_HEADER_POSTFIX_LEN (strlen("_nodes.idx"))
#define RELS_HEADER_POSTFIX_LEN (strlen("_relationships.idx"))
#define HEADER_FILE_POSTFIX_LEN (strlen(".idx"))

static phy_database*
phy_database_create_internal(char* db_name,
                             bool  open
#ifdef VERBOSE
                             ,
                             const char* log_file_name
#endif
)
{
    if (!db_name) {
        // LCOV_EXCL_START
        printf("physical database - create: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    phy_database* phy_db = calloc(1, sizeof(phy_database));

    if (!phy_db) {
        // LCOV_EXCL_START
        printf("physical database: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

#ifdef VERBOSE
    phy_db->log_file = fopen(log_file_name, "a+");

    if (!phy_db->log_file) {
        // LCOV_EXCL_START
        printf("physical database - create: failed to fopen %s: %s\n",
               log_file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
#endif

    char* catalogue_name =
          calloc(strlen(db_name) + CATALOGUE_POSTFIX_LEN + 1, sizeof(char));

    strncpy(catalogue_name, db_name, strlen(db_name) + 1);

    strncat(catalogue_name, ".info", CATALOGUE_POSTFIX_LEN);

    if (!open) {
        phy_db->catalogue = disk_file_create(catalogue_name
#ifdef VERBOSE
                                             ,
                                             log_file
#endif
        );
        disk_file_grow(phy_db->catalogue, 1);
    } else {
        phy_db->catalogue = disk_file_open(catalogue_name
#ifdef VERBOSE
                                           ,
                                           log_file
#endif
        );
    }

#ifdef ADJ_LIST
    char* header_name =
          calloc(strlen(db_name) + HEADER_POSTFIX_LEN + 1, sizeof(char));

    strncpy(header_name, db_name, strlen(db_name) + 1);

    strncat(header_name, ".idx", HEADER_POSTFIX_LEN);

    if (!open) {
        phy_db->header[0] = disk_file_create(header_name
#ifdef VERBOSE
                                             ,
                                             log_file
#endif
        );
    } else {
        phy_db->header[0] = disk_file_open(header_name
#ifdef VERBOSE
                                           ,
                                           log_file
#endif
        );
    }

    /* Create or open Record files */
    // +1 as strlen does not count \0
    char* record_file_name =
          calloc(strlen(db_name) + RECORD_FILE_POSTFIX_LEN + 1, sizeof(char));

    strncpy(record_file_name, db_name, strlen(db_name) + 1);

    strncat(rels_file_name, ".db", RECORD_FILE_POSTFIX_LEN);

    if (!open) {
        phy_db->records[0] = disk_file_create(record_file_name
#ifdef VERBOSE
                                              ,
                                              log_file
#endif
        );
    } else {
        phy_db->records[0] = disk_file_open(record_file_name
#ifdef VERBOSE
                                            ,
                                            log_file
#endif
        );
    }

#else
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

    if (!open) {
        phy_db->header[node_ft]         = disk_file_create(nodes_header_name
#ifdef VERBOSE
                                                   ,
                                                   log_file
#endif
        );
        phy_db->header[relationship_ft] = disk_file_create(rels_header_name
#ifdef VERBOSE
                                                           ,
                                                           log_file
#endif
        );
    } else {
        phy_db->header[node_ft]         = disk_file_open(nodes_header_name
#ifdef VERBOSE
                                                 ,
                                                 log_file
#endif
        );
        phy_db->header[relationship_ft] = disk_file_open(rels_header_name
#ifdef VERBOSE
                                                         ,
                                                         log_file
#endif
        );
    }

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

    if (!open) {
        phy_db->records[node_ft]         = disk_file_create(nodes_file_name
#ifdef VERBOSE
                                                    ,
                                                    log_file
#endif
        );
        phy_db->records[relationship_ft] = disk_file_create(rels_file_name
#ifdef VERBOSE
                                                            ,
                                                            log_file
#endif
        );
    } else {
        phy_db->records[node_ft]         = disk_file_open(nodes_file_name
#ifdef VERBOSE
                                                  ,
                                                  log_file
#endif
        );
        phy_db->records[relationship_ft] = disk_file_open(rels_file_name
#ifdef VERBOSE
                                                          ,
                                                          log_file
#endif
        );
    }
#endif

    bool valid_header;
    for (file_type ft = 0; ft < invalid_ft; ++ft) {
        /* check if record file is empty. */

        /* If it is empty, write an empty page to the header file. Alternatively
         * check if the header is one page long and has zero entries. */
        /* The first 8 byte encode the number of used bits. */
        if (!open || phy_db->records[ft]->file_size == 0) {
            valid_header = phy_database_validate_empty_header(phy_db, ft);

        } else {
            /* If the record file is not empty, the header is checked. */
            valid_header = phy_database_validate_header(phy_db, ft);
        }

        if (!valid_header) {
            // LCOV_EXCL_START
            printf("physical database: failed to open database - Invalid "
                   "header!\n");
            exit(EXIT_FAILURE);
            // LCOV_EXCL_STOP
        }
    }

    return phy_db;
}

phy_database*
phy_database_open(char* db_name
#ifdef VERBOSE
                  ,
                  const char* log_file_name
#endif
)
{
    return phy_database_create_internal(db_name,
                                        true
#ifdef VERBOSE
                                        ,
                                        log_file_name
#endif
    );
}

phy_database*
phy_database_create(char* db_name
#ifdef VERBOSE
                    ,
                    const char* log_file_name
#endif
)
{
    return phy_database_create_internal(db_name,
                                        false
#ifdef VERBOSE
                                        ,
                                        log_file_name
#endif
    );
}

void
phy_database_close(phy_database* db)
{
    if (!db) {
        // LCOV_EXCL_START
        printf("physical database - close: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    char* catalogue_fname = db->catalogue->file_name;
    disk_file_destroy(db->catalogue);
    free(catalogue_fname);

    char* header_fname;
    char* record_fname;
    for (file_type ft = 0; ft < invalid_ft; ++ft) {
        header_fname = db->header[ft]->file_name;
        record_fname = db->records[ft]->file_name;
        disk_file_destroy(db->header[ft]);
        disk_file_destroy(db->records[ft]);
        free(header_fname);
        free(record_fname);
    }

#ifdef VERBOSE
    if (fclose(db->log_file) != 0) {
        // LCOV_EXCL_START
        printf("disk file - destroy: Error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
#endif

    free(db);
}

void
phy_database_delete(phy_database* db)
{
    if (!db) {
        // LCOV_EXCL_START
        printf("physical database - delete: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    char* catalogue_fname = db->catalogue->file_name;
    disk_file_delete(db->catalogue);
    free(catalogue_fname);

    char* header_fname;
    char* record_fname;
    for (file_type ft = 0; ft < invalid_ft; ++ft) {
        header_fname = db->header[ft]->file_name;
        record_fname = db->records[ft]->file_name;
        disk_file_delete(db->header[ft]);
        disk_file_delete(db->records[ft]);
        free(header_fname);
        free(record_fname);
    }

#ifdef VERBOSE
    if (fclose(db->log_file) != 0) {
        // LCOV_EXCL_START
        printf("disk file - destroy: Error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
#endif
    free(db);
}

bool
phy_database_validate_empty_header(phy_database* db, file_type ft)
{

    /* first check if the header is also empty */
    if (db->header[ft]->file_size != 0) {
        /* if not, validate that it has zero entries and is one page long. */

        unsigned char buf[PAGE_SIZE];
        unsigned long size;

        read_page(db->catalogue, 0, buf);
        /* convert them to an unsigned long */
        memcpy(&size, buf + ft * sizeof(unsigned long), sizeof(unsigned long));

        /* if it has more than zero entries or is larger than a page,
         * the header is invalid */
        if (size != 0 || db->header[ft]->file_size != PAGE_SIZE) {
            printf("physical database - create: Too large header %s "
                   "for empty record file %s!\n",
                   db->header[ft]->file_name,
                   db->records[ft]->file_name);
            return false;
        }

        read_page(db->header[ft], 0, buf);

        for (size_t i = 0; i < PAGE_SIZE; ++i) {
            if (buf[i] != 0) {
                printf("physical database - create: Non-empty header %s "
                       "for empty record file %s!\n",
                       db->header[ft]->file_name,
                       db->records[ft]->file_name);
                return false;
            }
        }
    } else {
        /* if the header file is empty, write a page of zeros. */
        disk_file_grow(db->header[ft], 1);
    }
    /* Mark all bits as unsused */
    db->remaining_header_bits[ft] = PAGE_SIZE * CHAR_BIT;

    return true;
}

bool
phy_database_validate_header(phy_database* db, file_type ft)
{
    unsigned char buf[PAGE_SIZE];
    unsigned long size;

    /* first check if the header is empty */
    if (db->header[ft]->file_size == 0) {
        printf("physical database - create: Empty header %s for "
               "non-empty record file %s!\n",
               db->header[ft]->file_name,
               db->records[ft]->file_name);
        return false;
    }

    read_page(db->catalogue, 0, buf);
    /* convert them to an unsigned long */
    memcpy(&size, buf + ft * sizeof(unsigned long), sizeof(unsigned long));

    /* caluclate the number of slots in the record file */
    size_t n_slots = (db->records[ft]->file_size / SLOT_SIZE);

    /* check if the number of slots and the size of the header match */
    if (size != n_slots) {
        printf(
              "physical database - validate header: Missmatch in the number of "
              "slots and entries between header %s: %zu and record %s: %zu!\n",
              db->header[ft]->file_name,
              size,
              db->records[ft]->file_name,
              n_slots);
        return false;
    }

    db->remaining_header_bits[ft] = db->header[ft]->file_size * CHAR_BIT - size;
    return true;
}

void
allocate_pages(phy_database* db, file_type ft, size_t num_pages)
{
    if (!db ||
#ifdef ADJ_LIST
        ft != record_file
#else
        (ft != node_ft && ft != relationship_ft)
#endif
    ) {
        // LCOV_EXCL_START
        printf("physical database - allocate: Invalid arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    disk_file_grow(db->records[ft], num_pages);

    size_t neccessary_bits = num_pages * (PAGE_SIZE / SLOT_SIZE);

    if (db->remaining_header_bits[ft] < neccessary_bits) {
        size_t additional_bits =
              neccessary_bits - db->remaining_header_bits[ft];
        size_t additional_bytes =
              (additional_bits / CHAR_BIT) + (additional_bits % CHAR_BIT != 0);
        size_t additional_pages = (additional_bytes / PAGE_SIZE)
                                  + (additional_bytes % PAGE_SIZE != 0);

        disk_file_grow(db->header[ft], additional_pages);

        db->remaining_header_bits[ft] =
              (additional_pages * PAGE_SIZE * CHAR_BIT - additional_bits);
    } else {
        db->remaining_header_bits[ft] -= neccessary_bits;
    }

    /* Write the size back to the catalogue */

    unsigned char catalogue[PAGE_SIZE];
    size_t        bits;

    read_page(db->catalogue, 0, catalogue);

    memcpy(
          &bits, catalogue + ft * sizeof(unsigned long), sizeof(unsigned long));

    bits += neccessary_bits;

    memcpy(
          catalogue + ft * sizeof(unsigned long), &bits, sizeof(unsigned long));
    write_page(db->catalogue, 0, catalogue);
}

// LCOV_EXCL_START
void
deallocate_pages(phy_database* db, file_type ft)
{
    if (!db) {
        printf("physical database - deallocate page: Invalid "
               "arguments!\n");
        exit(EXIT_FAILURE);
    }

    printf("pyhsical database - dealloc: Not Implemented! %p, %i\n", db, ft);
    exit(EXIT_FAILURE);
}

void
physical_database_defragment(phy_database* pdb)
{
    printf("pyhsical database - defragment: Not Implemented! %p\n", pdb);
    exit(EXIT_FAILURE);
}
// LCOV_EXCL_STOP
