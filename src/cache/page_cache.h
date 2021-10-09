/*!
 * \file page_cache.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief Represents a page buffer or cache, to avoid reading and writing to
 * disk directly on every access. Instead keeps an array of pages in
 * preallocated memory locations called frames. Pages that are already in memory
 * do not need to be loaded and pages are only written when they are victed from
 * the cache. This happens if there is no free frame left and a page needs to be
 * loaded which is currently not in memory. Pin makes a page available and unpin
 * signals the end of the local usage. Evict selects victim pages to write back
 * to disk and flush actually writes them back to disk. new_page provides the
 * ability to allocate a new record page and pin it.
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stddef.h>

#include "constants.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "page.h"
#include "physical_database.h"

/*! \struct page_cache
 *
 * Represents a page buffer or cache, to avoid reading and writing to disk
 * directly on every access. It consists of a pointer to a physical database and
 * an array of \ref page instances. Changes to a page are only flushed to disk
 * when a pages is evicted from the page cache, i.e. it is written to disk and
 * another page from disk is stored into the frame.
 * If a page is read or written it needs to be pinned before and unpinned after
 * the operations.
 */
typedef struct
{
    /*! The physical database from and to which data is read and written. */
    phy_database* pdb;
    /*!  the amount of frames that shall be available for caching pages. */
    size_t n_frames;
    /*! Counter for the total number of pins. */
    size_t num_pins;
    /*! Counter for the total number of unpins. */
    size_t num_unpins;
    /*! Flag that changes the eviction strategy. Used for importing datasets. */
    bool bulk_import;
    /*! A log file to log pin, unpin, evict and flush calls to. */
    FILE* log_file;
    /*! A linked list of free frames, that is used when pinning pages. */
    llist_ul* free_frames;
    /*! A queue of the most recently unpinned pages. User for eviction (LRU). */
    queue_ul* recently_referenced;
    /*! A dictionary for each \ref file_kind and \ref file_type that maps from
     * page number to frame number. */
    dict_ul_ul* page_map[invalid][invalid_ft];
    /*! The frames to cache pages. */
    page** frames;
} page_cache;

/*!
 *  Constructor for the page_cache struct.
 *  Allocates memory for the struct, sets the \ref phy_database, allocates
 * consecutive memory for the frames, creates the (currently empty) pages and
 * adds all frames to the empty frame linked list. Creates the page_no to
 * frame_no mapping dictionaries and opens the log file specified by \p
 * log_path.
 *
 *  \param pdb The physical database that shall be read from and written to.
 *  \param n_frames The numer of frames to cache pages in.
 *  \param log_path The path for a log file to log pin, unpin, evict and flush
 * calls to. \return A pointer to an initialized page_cache struct.
 */
page_cache*
page_cache_create(phy_database* pdb, size_t n_frames, const char* log_path);

/*!
 *  Destructor for the page_cache struct.
 *  Flushes all pages, destroys the free frame list and the recently referenced
 * queue, as well as the page_no to frame_no map. Also frees the pages and the
 * buffer used for these. Closes the log file and frees the page_cache struct.
 *
 *  \param pc The page_cache struct to be destructed.
 */
void
page_cache_destroy(page_cache* pc);

/*!
 *  Pins a page so that it can be used.
 *  This includes looking up if the page is already in-memory, finding a free
 * frame or evicting pages to free frames and finally reading it from the
 * correct disk file if it's not present. The \ref page 's pin counter is
 * incremented by one, file_kind, file_type and page_no of the page are set and
 * the dirty flag is unset. Also increments the total pin counter.
 *
 * \param pc A pointer to the page_cache that shall bring the page in-memory.
 * \param page_no The number of the page to be accessed.
 * \param fk The file_kind of the page.
 * \param ft The file_type of the page.
 * \param log A flag indicating wether the pin should be logged. If so the
 * format is "Pin fk ft page_no".
 * \return A pointer to the page that was requested.
 */
page*
pin_page(page_cache* pc, size_t page_no, file_kind fk, file_type ft, bool log);

/*!
 * Unpins a page after it has been accessed and is not needed anymore.
 * Finds the frame for the page and decrements its page pin count by one. Moves
 * the frame to the back of the recently referenced queue, increments to total
 * unpin counter.
 *
 * \param pc A pointer to the page_cache that has the page in-memory.
 * \param page_no The number of the page to be unpinned.
 * \param fk The file_kind of the page.
 * \param ft The file_type of the page.
 * \param log A flag indicating wether the unpin should be logged. If so the
 * format is "Unpin fk ft page_no".

 */
void
unpin_page(page_cache* pc,
           size_t      page_no,
           file_kind   fk,
           file_type   ft,
           bool        log);

/*!
 * Evicts a page from it's frame in the page_cache.
 * If a page is to be pinned and there are no free frames left, then another
 * page needs to be removed from its frame. Evict does that using the Least
 * Recently Used K algorithm. This algorithm always evicts the up to K least
 * recently unpinned pages that have a page pin count of 0. It adds the up to K
 * removed frames to the free frames list and removes them from the recently
 * referenced queue. it also flushes these pages if neccessary.
 *
 * \param pc The page cache to evict a page from.
 * \param log A flag indicating if the evcition shall be logged. If so the
 * format is "Evict fk ft page_no".
 */
void
evict(page_cache* pc, bool log);

/*!
 * Evicts all pages from their frame in the page_cache if possible.
 * If a page is to be pinned and there are no free frames left, then another
 * page needs to be removed from its frame. Bulk evict does that By evicting all
 * pages which have a pin count of 0. This is useful for importing data sets as
 * the recently referenced queue does not have to be handled which can save
 * quite some cycles when the dataset grows larger and as the reuse when
 * importing is not neccessarily as large as when querying with outher payloads.
 * It adds the removed frames to the free frames list and removes them from the
 * recently referenced queue. it also flushes these pages if neccessary.
 *
 * \param pc The page cache to evict a page from.
 */
void
bulk_evict(page_cache* pc);

/*!
 *  Writes the page to the appropriate disk file if it's dirty.
 *  Also unsets the dirty flag of the page.
 *
 *  \param pc The page cache holding the frame to be flushed.
 *  \param frame_no The number of the frame to be flushed.
 *  \param log A flag indicating if the flushing operation shall be logged. If
 * so the format is "Flushed fk ft page_no"
 */
void
flush_page(page_cache* pc, size_t frame_no, bool log);

/*!
 * A wrapper arroung the flush_page function to apply it to all frames.
 * \param pc The page cache to be flushed.
 * \param log A flag indicating if the flush operations shall be logged.
 */
void
flush_all_pages(page_cache* pc, bool log);

/*!
 *  Allocates and pins a new record page.
 *  Calls \ref allocate_pages() and pin_page().
 *
 *  \param pc The page cache where the new record page should be pinned.
 *  \param ft The type of the record to allocate a page for see \ref file_type.
 *  \param log A flag indicating if the subsequent calls shall be logged.
 *  \return A pointer to the just allocated record page.
 */
page*
new_page(page_cache* pc, file_type ft, bool log);

/*!
 * Swaps the log file of the page cache.
 *
 * \param pc The page cache whichs log file to swap.
 * \param log_file_path The path of the new log file.
 */
void
page_cache_swap_log_file(page_cache* pc, const char* log_file_path);

/*!
 * This function changes the number of frames that are available to the page
 * cache. It frees the previously allocated memory for the pages' data buffer
 * and initializes a new one of size n_frames. It also reinitializes the free
 * frames list, the recently referenced queue and the page map.
 *
 * \param pc The page cache whichs size shall be changed.
 * \param n_frames The new size of the page cache in number of frames (i.e.
 * PAGE_SIZE x n_pages)
 */
void
page_cache_change_n_frames(page_cache* pc, size_t n_frames);

#endif
