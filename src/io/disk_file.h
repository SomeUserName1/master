/*!
 * \file disk_file.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief The here declared struct and functions are used to represent a file on
 * disk, to read and to write page oriented to the file as well as growing and
 * shrinking the file.
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
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
 *
 * The disk_file struct represents a file on disk.
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

/*!
 * Creates a new file at the specified path and calls the actual constructor
 * disk_file_open().
 *
 * \param file_name the path of the file.
 * \param log_file a log file to log read and write operations to.
 * \return A pointer to an initialized disk_file struct.
 */
disk_file*
disk_file_create(char* file_name, FILE* log_file);

/*!
 * Constructor for the disk_file struct.
 * Allocates the struct, opens the file specified by file_name for reading and
 * writing binary, allocates and sets a buffer used by the OS that is 8 times as
 * large as a page, initializes the file size in bytes and pages, sets the read
 * and write counters to zero and stores the FILE* in the struct.
 *
 * \param file_name the path of the file.
 * \param log_file a log file to log read and write operations to.
 * \return A pointer to an initialized disk_file struct.
 */
disk_file*
disk_file_open(char* file_name, FILE* log_file);

/*!
 * Destructor for the disk_file struct, leaving the file on disk.
 * Closes the file, frees the OS buffer and the struct itself.
 *
 * \param df The disk_file struct to be destructed.
 */
void
disk_file_destroy(disk_file* df);

/*!
 * Destructor for the disk_file struct, deleting the file from disk.
 * Closes and removes the file, frees the OS buffer and the struct itself.
 *
  \param df The disk_file struct to be destructed and whichs file is to be
 deleted.
 */
void
disk_file_delete(disk_file* df);

/*!
 * This function is a wrapper to read a single page. It calls read_pages()
 * with the same parameter for first page and last page.
 *
 * \param df The disk_file to read the page from.
 * \param page_no The number of the page that shall be read.
 * \param buf A buffer into which the contents of the page are loaded. Needs to
 * match the size of a page.
 * \param log A flag indicating wether or not the read
 * should be logged to the log file mentioned in the disk_file struct.
 */
void
read_page(disk_file* df, size_t page_no, unsigned char* buf, bool log);

/*!
 * Read a range of pages from the disk_file.
 * The range includes the first and the last page. First, the file postition of
 * the first page is seeked. Then \p lst_page - \p fst_page + 1 pages are read
 * and the read counter is incremented. If the \p log flag is set, the read
 * operation is logged in the format "read_pages, file_name fst_page lst_page".
 *
 * \param df The disk_file to read the pages from.
 * \param fst_page The number of the first page to be read.
 * \param lst_page The number of the last page to be read.
 * \param buf A buffer into which the contents of the pages are loaded. It needs
 * to be sufficiently large to hold the corresponding amount of pages and needs
 * to be aligned with the page size.
 * \param log A flag indicating wether or not
 * the read should be logged to the log file mentioned in the disk_file struct.
 */
void
read_pages(disk_file*     df,
           size_t         fst_page,
           size_t         lst_page,
           unsigned char* buf,
           bool           log);

/*!
 * This function is a wrapper to write a single page. It calls write_pages()
 * with the same parameter for first page and last page.
 *
 * \param df The disk_file to write the page to.
 * \param page_no The number of the page that shall be written.
 * \param data A buffer holding the contents of the page to write. Needs to
 * match the size of a page.
 * \param log A flag indicating wether or not the write
 * should be logged to the log file mentioned in the disk_file struct.
 */
void
write_page(disk_file* df, size_t page_no, unsigned char* data, bool log);

/*!
 * Write a range of pages to the disk_file.
 * The range includes the first and the last page. First, the file postition of
 * the first page is seeked. Then \p lst_page - \p fst_page + 1 pages are
 * written and the write counter is incremented. If the \p log flag is set, the
 * read operation is logged in the format "write_pages, file_name fst_page
 * lst_page".
 *
 * \param df The disk_file to read the pages from.
 * \param fst_page The number of the first page to be read.
 * \param lst_page The number of the last page to be read.
 * \param data A buffer that holds the contents of the pages to be written. It
 * needs to be sufficiently large to hold the corresponding amount of pages and
 * needs to be aligned with the page size.
 * \param log A flag indicating wether
 * or not the write should be logged to the log file mentioned in the disk_file
 * struct.
 */
void
write_pages(disk_file*     df,
            size_t         fst_page,
            size_t         lst_page,
            unsigned char* data,
            bool           log);

/*!
 * This function syncs the contents of the file from the OS buffer to the file
 * on disk. This is one of three Unix-dependent functions besides
 * disk_file_shrink() and the constant PAGE_SIZE that gathers the page size
 * from the OS. It uses the fileno function from unistd.h to fetch the file
 * descriptor from the file pointer. It then calls fsync.
 *
 * \param df The disk file to be synced.
 */
void
disk_file_sync(disk_file* df);

/*!
 * Clears the contents of a page on disk by writing zeros to the file at the
 * appropriate postition. The function calls write_page() with the corresponding
 * \p page_no and a buffer of the size of a page, that is initialized to 0.
 *
 * \param df The disk_file that contains the page to be cleared.
 * \param page_no The number of the page to be cleared.
 * \param log A flag indicating wether this clear operation should be logged.
 */
void
clear_page(disk_file* df, size_t page_no, bool log);

/*!
 * Grows a disk file by \p by_num_pages pages by appending zeros to the file.
 * First the function seeks to the end of the file. Then it allocates \p
 * by_num_pages pages ehich are initialized to zero and writes them. Finally the
 * write cound is incremented, the file_size and the num_pages in the disk_file
 * struct are updated and if the log flag is set the operation is logged.
 *
 * \param df The disk_file to grow.
 * \param by_num_pages The number of additional pages that the file should have.
 * \param log A flag indicating wether the operation is logged. If so, the
 * format is "grow_file file_name by_num_pages".
 */
void
disk_file_grow(disk_file* df, size_t by_num_pages, bool log);

/*!
 * Shrinks the file by \p by_num_pages pages.
 * This is the second function that is Unix-specific as ftruncate is called.
 * ftruncate requires the OS buffers to be flushed to disk before it can operate
 * as expected. Thus, first the OS buffers are flushed to disk. Then a file
 * descriptor is acquired similarly as in disk_file_sync(). Then ftruncate is
 * called to adjust the size. Finally the file size and number of pages are
 * updated in the disk_file struct, the write count is incremented and if the
 * log flag is set, the operation is logged.
 *
 * DANGER ZONE!
 * This method assumes that the empty pages have
 * been moved to the end. It will simply shrink the
 * file by cutting of the last by_no_pages pages.
 * If these are not empty, the records on these
 * pages will be lost!

 *
 * \param df The disk file to be shrinked.
 * \param by_num_pages The amount of pages that the disk file shall be shrinked
 * by.
 * \param log A flag indicating if the operation shall be logged. If so, the
 * format is "shrink_file file_name by_num_pages".
 */
void
disk_file_shrink(disk_file* df, size_t by_num_pages, bool log);

/*!
 * Swaps the file that reads and writes are logged to.
 * In order to change the log file of a physical database you should call \ref
 * phy_database_swap_log_file() instead.
 *
 * \param df The disk file whichs log file is to be changed.
 * \param new_log_file The file to log future reads and writes to.
 */
void
disk_file_swap_log_file(disk_file* df, FILE* new_log_file);
#endif
