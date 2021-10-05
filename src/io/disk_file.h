/*!
 * \file disk_file.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief The here declared struct and functions are used to represent a file on
 * disk, to read and to write page oriented to the file as well as growing and
 * shrinking the file.
 *
 * \copyright Copyright (c) 2021- University of Konstanz. \
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef DISK_FILE_H
#define DISK_FILE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*! \struct disk_file
 * \brief The disk_file struct represents a file on disk.
 *
 * This includes its name, the associated FILE*, the size of the file in bytes,
 * the number of pages the file consists of, counters for applied read and write
 * operations, the OS buffer that is used and a FILE* to a log file, that logs
 * reads and writes.
 */
typedef struct
{
    char*  file_name;   /*!< The name of the file on disk. */
    FILE*  file;        /*!< The FILE* associated with the opened file. */
    size_t file_size;   /*!< The size of the file in bytes. */
    size_t num_pages;   /*!< The size of the file in pages. */
    size_t read_count;  /*!< A counter for applied read operations. */
    size_t write_count; /*!< A counter for applied write operations. */
    char*  f_buf;       /*!< The buffer that is used by the operating system. */
    FILE*  log_file;    /*!< A FILE* to log read and write operations to. */
} disk_file;

/*! \fn disk_file* disk_file_create(char* file_name, FILE* log_file);
 * Creates a new file at the specified path and calls the actual constructor
 * disk_file_open().
 * \param file_name the path of the file.
 * \param log_file a log file to log read and write operations to.
 * \return A pointer to an initialized disk_file struct.
 */
disk_file*
disk_file_create(char* file_name, FILE* log_file);

/*! \fn disk_file* disk_file_open(char* file_name, FILE* log_file);
 * Constructor for the disk_file struct.
 * Allocates the struct, opens the file specified by file_name for reading and
 * writing binary, allocates and sets a buffer used by the OS that is 8 times as
 * large as a page, initializes the file size in bytes and pages, sets the read
 * and write counters to zero and stores the FILE* in the struct.
 * \param file_name the path of the file.
 * \param log_file a log file to log read and write operations to.
 * \return A pointer to an initialized disk_file struct.
 */
disk_file*
disk_file_open(char* file_name, FILE* log_file);

/*! \fn  void disk_file_destroy(disk_file* df);
 * Destructor for the disk_file struct, leaving the file on disk.
 * Closes the file, frees the OS buffer and the struct itself.
 * \param df The disk_file struct to be destructed.
 */
void
disk_file_destroy(disk_file* df);

/*! \fn void disk_file_delete(disk_file* df);
 * Destructor for the disk_file struct, deleting the file from disk.
 * Closes and removes the file, frees the OS buffer and the struct itself.
  \param df The disk_file struct to be destructed and whichs file is to be
 deleted.
 */
void
disk_file_delete(disk_file* df);

void
read_page(disk_file* df, size_t page_no, unsigned char* buf, bool log);

void
read_pages(disk_file*     df,
           size_t         fst_page,
           size_t         lst_page,
           unsigned char* buf,
           bool           log);

void
write_page(disk_file* df, size_t page_no, unsigned char* data, bool log);

void
write_pages(disk_file*     df,
            size_t         fst_page,
            size_t         lst_page,
            unsigned char* data,
            bool           log);

void
disk_file_sync(disk_file* df);

void
clear_page(disk_file* df, size_t page_no, bool log);

void
disk_file_grow(disk_file* df, size_t by_num_pages, bool log);

void
disk_file_shrink(disk_file* df, size_t by_num_pages, bool log);

#endif
