/*!
 * \file page.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief The here declared struct and functions are used to represent a  page
 * as a buffer in memory, holding the contents of pages loaded from disk. It
 * provides functions to read and write binary data from and to the buffer.
 *
 * \copyright Copyright (c) 2021- University of Konstanz. \
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <stdio.h>

#include "physical_database.h"

/*!
 *  \struct page
 *
 *  A page is a collection of metadata along with a buffer that is exactly as
 * large as a page. The metadata include information to uniquely identify from
 * which file and position the page is actually loaded from or to be written to.
 * This includes the \ref file_kind, the \ref file_type and the number of the
 * page. Additionally there is a pin_count which is used by the \ref page_cache
 * and a dirty flag indicating if this page contains changes that needs be be
 * written to disk.
 */
typedef struct
{
    /*! The file_kind of the page (one of catalogue, header, record). */
    file_kind fk;
    /*! The file_type of the page (one of node_ft, relationship_ft). */
    file_type ft;
    /*! The number of the page. */
    size_t page_no;
    /*! A counter specifying how often the page is currently pinned (\ref
     * pin_page()). */
    unsigned int pin_count;
    /*! A flag indicating if the page contains changes to be synced back to
     * disk. */
    bool dirty;
    /*! A buffer that is PAGE_SIZE bytes large and holds the contents of a
     * page from disk. */
    unsigned char* data;
} page;

/*!
 *  Constructor for page.
 *  Allocates memory for the struct, sets fk, ft, page_no to invalid values
 * (\ref invalid, \ref invalid_ft, ULONG_MAX) and sets the pointer to the
 * provided buffer.
 *
 *  \param data A buffer that is exactly PAGE_SIZE bytes large.
 *  \return A pointer to an initialized page struct.
 */
page*
page_create(unsigned char* data);

/*!
 * Destructor for page.
 * Checks if the pin_count is zero and that the page is not dirty. Frees the
 * struct then. NOTE: This does not free the buffer. Providing the buffer as
 * parameter in the constructor and not freeing it in the destructor is
 * necessary to allow for many pages with consecutive buffers (neccessary for
 * bulk ops).
 *
 * \param p The page struct to be destructed.
 */
void
page_destroy(page* p);

/*!
 *  Reads an unsigned long from the buffer of the page \p p at a certain \p
 * offset.
 *
 *  \param p The page to read from
 *  \param offset The offset from the start of the page in bytes.
 *  \return the unsigned long read from the page at the offset.
 */
unsigned long
read_ulong(page* p, size_t offset);

/*!
 *  Writes an unsigned long to the buffer of the page \p p at a certain \p
 * offset.
 *
 *  \param p The page to write to.
 *  \param offset The offset from the start of the page in bytes.
 *  \param value the unsigned long write to the page.
 */
void
write_ulong(page* p, size_t offset, unsigned long value);

/*!
 *  Reads an unsigned char from the buffer of the page \p p at a certain \p
 * offset.
 *
 *  \param p The page to read from
 *  \param offset The offset from the start of the page in bytes.
 *  \return the unsigned char read from the page at the offset.
 */
unsigned char
read_uchar(page* p, size_t offset);

/*!
 *  Writes an unsigned char to the buffer of the page \p p at a certain \p
 * offset.
 *
 *  \param p The page to write to.
 *  \param offset The offset from the start of the page in bytes.
 *  \param value the unsigned char write to the page.
 */
void
write_uchar(page* p, size_t offset, unsigned char value);

/*!
 *  Reads a double from the buffer of the page \p p at a certain \p offset.
 *
 *  \param p The page to read from
 *  \param offset The offset from the start of the page in bytes.
 *  \return the double read from the page at the offset.
 */
double
read_double(page* p, size_t offset);

/*!
 *  Writes a double to the buffer of the page \p p at a certain \p
 * offset.
 *
 *  \param p The page to write to.
 *  \param offset The offset from the start of the page in bytes.
 *  \param value the double write to the page.
 */
void
write_double(page* p, size_t offset, double value);

/*!
 *  Reads a string from the buffer of the page \p p at a certain \p offset.
 *
 *  \param p The page to read from
 *  \param offset The offset from the start of the page in bytes.
 *  \param buf the strinf of length \p len read from the page \p p at the \p
 * offset is written to this buffer.
 *  \param len The length of the string.
 */
void
read_string(page* p, size_t offset, char* buf, size_t len);

/*!
 *  Writes a string of length \p len to the buffer of the page \p p at a certain
 * \p offset.
 *
 *  \param p The page to write to.
 *  \param offset The offset from the start of the page in bytes.
 *  \param value the '\0' terminated char* to write to the page.
 *  \param len The length of the string.
 */
void
write_string(page* p, size_t offset, char* value, size_t len);

/*!
 *  Prints the metadata of the page struct.
 *
 *  \param p The page to print.
 */
void
page_pretty_print(const page* p);

#endif
