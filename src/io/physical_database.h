/*!
 * \file physical_database.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief The here declared struct and functions are used to represent the
 * physical part of the database, i.e. the record, header and catalogue files
 * necessary to mirror the architecture of Neo4J. In particular having extra
 * files for nodes and relationships and extra headers for each of them makes
 * this necessary.
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <stddef.h>

#include "disk_file.h"

/*! \enum file_type
 *
 *  The file_type encodes if the referenced disk file carries node or
 * relationship records or is the respective header.
 */
typedef enum
{
    /*! Indicates that the file is storing or addressing nodes. */
    node_ft,
    /*! Indicates that the file is storing or addressing relationships. */
    relationship_ft,
    /*! Is used to iterate, as "NULL" value and to validate parameters. */
    invalid_ft
} file_type;

/*! \enum file_kind
 *
 *  The file kind encodes if the file holds records or header bitmaps or the
 * system catalogue.
 * The terms type and kind are used informally and are not related to
 * concepts from functional programming theory.
 */
typedef enum
{
    /*!
     * The system catalogue currently only stores the the number of slots that
     * the respective record files have.
     */
    catalogue,
    /*!
     * The header files store for each record type a bitmap, where one bit
     * corresponds to one slot.
     */
    header,
    /*!
     * The record files store the actual records in binary format.
     * They are divided into slots of 16 bytes. This is done to account
     * for different record sizes of nodes and edges, avoiding code duplication
     * when it comes to the allocation of additional record pages.
     */
    records,
    /*! Used for iterations, as "NULL" value and to validate parameters. */
    invalid
} file_kind;

/*! \struct phy_database
 *
 * This struct represents the persistent parts of the database: All files that
 * are needed for a database instance.
 */
typedef struct
{
    /*! The system catalogue of the system, see #file_kind. */
    disk_file* catalogue;
    /*! One header for each record file, see #file_kind. */
    disk_file* header[2];
    /*! One record file for each record type (nodes, relationships), see
     * #file_kind. */
    disk_file* records[2];
    /*! When allocating a header page, not neccessarily enough record pages are
     * available to be mapped by the header. Thus the remaining_header_bits
     * field stores the amount of bits that are not in use. Imagine allocating a
     * single record page. With a page size of 4 KiB, it has 256 slots, so the
     * header page only needs to address 256 slots and thus only 256 of the 4096
     * x 8 bits are in use. For details on header page allocation, see
     * allocate_page().  */
    size_t remaining_header_bits[2];
    /*! A FILE*, that is used for logging at the file level. This is passed
     * through to the disk_filestructs. */
    FILE* log_file;
} phy_database;

/*!
 * Constructor for phy_database, creating all necessary files.
 * Allocates memory for the stuct itself, opens the log file from the path \p
 * log_file, assigns each file the name and an ending: ".info" for the
 * catalogue, ".idx" for the headers and ".db" for records. The header and
 * record file also contain either "nodes" or "relationships" before the suffix.
 * It then creates the disk files and validates the empty header (see
 * phy_database_validate_empty_header()).
 *
 * \param db_name The base name of the database.
 * \param log_file The path where the underlying disk files shall log their
 * operations to, if the log flag is set in the respective operations.
 * \return A pointer to an initialized phy_database struct with newly created
 * files on disk.
 */
phy_database*
phy_database_create(char* db_name, const char* log_file);

/*!
 * Constructor for phy_database, opening the files of a previously created and
 * closed database. Allocates memory for the stuct itself, opens the log file
 * from the path \p log_file, assigns each file the name and an ending: ".info"
 * for the catalogue, ".idx" for the headers and ".db" for records. The header
 * and record file also contain either "nodes" or "relationships" before the
 * suffix. It then opens the disk files and validates the  header (see
 * phy_database_validate_header() and phy_database_validate_empty_header()).
 *
 * \param db_name The base name of the database.
 * \param log_file The path where the underlying disk files shall log their
 * operations to, if the log flag is set in the respective operations.
 * \return A pointer to an initialized phy_database struct with the files of the
 * specified database opened.
 */
phy_database*
phy_database_open(char* db_name, const char* log_file);

/*!
 *  Deletes the physical database, especially the underlying files on disk.
 *  Deletes all disk files by calling \ref disk_file_delete() for each, closes
 * the log file, frees the char* used for the file names.
 *
 *  \param db The physical database to delete.
 */
void
phy_database_delete(phy_database* db);

/*!
 *  Closes the physical database.
 *  Closes all disk files by calling \ref disk_file_destroy() for each, closes
 * the log file, frees the char* used for the file names.
 *
 *  \param db The physical database to delete.
 */
void
phy_database_close(phy_database* db);

/*!
 *  Validates that a header is empty.
 *  Assumes that there exist 0 records for the \p ft, so that no page has been
 * allocated for records yet. That means that the header is either 0 bytes large
 * or that both the system catalogue stores exactly zero entries and the header
 * has the size of a page and all bytes are set to zero. In case that the header
 * is 0 bytes large, the file is grown by one (empty) page. Finally the number
 * of remaining bits (all bits on the page) are set.
 *
 * \param db The phy_database struct, which holds the header.
 * \param ft The file type of the header to be checked (node or relationship,
 * see \ref file_type).
 * \return true if the header is valid, false if it isn't.
 */
bool
phy_database_validate_empty_header(phy_database* db, file_type ft);

/*!
 *  Validates a non-empty header.
 *  Assumes that at least one page in the respective header file (\p ft) has
 * been allocated. That means that the header is larger than 0 bytes, that the
 * number of slots stored in the system catalogue corresponds to the number of
 * slots in the record file. Finally the number of remaining bits are set.
 *
 * \param db The phy_database struct, which holds the header.
 * \param ft The file type of the header to be checked (node or relationship,
 * see \ref file_type).
 * \return true if the header is valid, false if it isn't.
 */
bool
phy_database_validate_header(phy_database* db, file_type ft);

/*!
 * This function takes care of allocation new record pages for a specified
 * file_type \p ft, i.e. growing the record file by \p num_pages and if
 * necessary also allocating new header pages. In contrast to \ref
 * disk_file_grow(), this function handles both record files and headers as a
 * dependent entity instead of independently growing them.
 *
 * \param db The phy_database that shall be grown.
 * \param ft The type (node or relationship) of the record and header files to
 * be grown. \param num_pages By how many pages the record file shall be grown.
 * \param log A flag indicating if the allocation operation shall be logged on
 * the read/write page level.
 */
void
allocate_pages(phy_database* db, file_type ft, size_t num_pages, bool log);

#endif
