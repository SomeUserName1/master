#ifndef LRU_H
#define LRU_H

#include "defs.h"
#include "page_queue.h"

#include <unordered_map> // hash map

/**
 * Uses array of ints, for example produced by PageQueue, 
 * as input for pages to read.
 */
class LRUBuffer
{
public:
	LRUBuffer(unsigned int size_b = (1<<20));
	~LRUBuffer(void);

	/**
	 * Reads the given pages from disk into buffer.
	 * Reads intervals at a time, ideally all pages in one 
	 * sequential read if all page ids contiguous.
	 * @param pages, array of page ids of length k
	 * @param k, length of pages
	 */
	void fill(int* pages, int k);
	
	/**
	 * Returns pointer into buffer for the given page id.
	 * @param page_id, page identifier
	 * @return pointer to page data
	 */
	char* get_page_by_pid(b_id page_id);
	
	/**
	 * Returns pointer into buffer for the given buffer slot index.
	 * @param slot_index, index in buffer
	 * @return pointer to page data
	 */
	char* get_page_by_slot(int slot_index);
	
	/**
	 * @param page_id, page identifier
	 * @return true if page with page_id is in buffer
	 */
	bool contains(b_id page_id);

	/**
	 * @param slot_index, index in buffer
	 * @return true if given buffer index contains a page
	 */
	bool slot_occupied(int slot_index);
	
	/**
	 * @param slot_index, index in buffer
	 * @return page id of page in slot_index in buffer; -1 if empty
	 */
	b_id get_page_id(int slot_index);
	
	/**
	 * @return the number of pages that the buffer can hold
	 */
	int get_num_slots();

	int get_seq_reads(int num);

private:
	char* buffer;
	int lru; // least recently added index

	// page_id -> buf_index
	std::unordered_map<b_id, int>* page_lookup;
	// buf_index -> page_id
	std::unordered_map<int, b_id>* buf_lookup;

	void set_maps(int* pages, int buf_offset, int page_offset, int num);
};

#endif
