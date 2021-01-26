#include "lru.h"
#include "memory_mgr.h"

#include <assert.h>

int buf_size_slots;
int find_next_interval(int* pages, int k, int start_index);

int* seq_reads;

LRUBuffer::LRUBuffer(unsigned int buf_size_bytes) : lru(0)
{
	//assert (buf_size_bytes % GLOBALS::blk_size == 0);
	buf_size_slots = buf_size_bytes / GLOBALS::blk_size;
	buffer = new char[buf_size_bytes];

	page_lookup = new std::unordered_map<b_id, int>();
	buf_lookup = new std::unordered_map<int, b_id>();

	seq_reads = new int[get_num_slots()];
	for (int i = 0; i < get_num_slots(); ++i)
		seq_reads[i] = 0;
}

LRUBuffer::~LRUBuffer()
{
	delete seq_reads;
	page_lookup->clear();
	buf_lookup->clear();
	delete page_lookup;
	delete buf_lookup;
	delete buffer;
}

int LRUBuffer::get_seq_reads(int num)
{
	if (num > 0)
		return seq_reads[num-1];
	else
		return 0;
}

void LRUBuffer::fill(int* pages, int k)
{
	if (k > buf_size_slots)
		return;

	int p_start = -1;
	int p_end = -1;
	int p_interval = -1; // page-related variables
	
	int b_right = 0; // num of slots to the right of lru index
	int b_num = 0; // num of slots to read into buffer

	int total_pages_read = 0;

	while (total_pages_read < k) {
		// read one interval
		int pages_read = 0;
		p_start = p_end+1;
		p_end = find_next_interval(pages, k, p_start);
		p_interval = p_end - p_start + 1; // +1 since inclusive both ends

		for (int i = 0; i < p_interval; ++i)
			++seq_reads[i];

		//printf("Reading interval %d to %d.\n", p_start, p_end);

		b_right = buf_size_slots - lru;

		if (b_right > p_interval)
			b_num = p_interval;
		else
			b_num = b_right;

		set_maps(pages, lru, p_start, b_num);
		mem_read_blocks(buffer+(lru*GLOBALS::blk_size), pages[p_start], b_num);
		pages_read += b_num;
		lru += b_num;

		// if each end of buffer
		if (pages_read < p_interval) {
			lru = 0; // start filling buffer from 0
			b_num = p_interval - pages_read;

			set_maps(pages, lru, p_start+pages_read, b_num);
			mem_read_blocks(buffer, pages[p_start+pages_read], b_num);
			pages_read += b_num;
			lru += b_num;
		}

		// assert whole interval read
		// assert (pages_read == p_interval);

		total_pages_read += p_interval;
	}
	// assert (total_pages_read == k);
}

/*
 * @return end of interval
 */
int find_next_interval(int* pages, int k, int start_index)
{
	if (start_index+1 == k) // no possibility of interval
		return start_index;

	int prev = start_index;
	for (int next = start_index+1; next < k; ++next) {
		if (pages[next] == pages[prev]+1) {
			++prev;
		} else {
			break;
		}
	}
	return prev;
}

void LRUBuffer::set_maps(int* pages, int buf_offset, int page_offset, int num)
{
	for (int i = 0; i < num; ++i) {
		b_id page_to_read = pages[page_offset+i];
		int slot_to_fill = buf_offset+i;
		//printf("Read page %d.\n", page_to_read);
		
		int curr_page = (*buf_lookup)[slot_to_fill];
		bool existed = (page_lookup->find(curr_page) == page_lookup->end()) ? false : true;
		page_lookup->erase(curr_page);
		if (existed)
			assert(page_lookup->find(curr_page) == page_lookup->end());

		(*page_lookup)[page_to_read] = slot_to_fill;
		(*buf_lookup)[slot_to_fill] = page_to_read;
	}
}

int LRUBuffer::get_num_slots()
{
	return buf_size_slots;
}

char* LRUBuffer::get_page_by_pid(b_id page_id)
{
	if (page_lookup->find(page_id) == page_lookup->end())
		fill(&page_id, 1);
	int slot = (*page_lookup)[page_id];
	//char* page = new char[GLOBALS::blk_size];
	//memcpy(page, buffer+(slot*GLOBALS::blk_size), GLOBALS::blk_size);
	//return page;
	return buffer+(slot*GLOBALS::blk_size);
}

char* LRUBuffer::get_page_by_slot(int slot_index)
{
	if (slot_index >= buf_size_slots)
		return NULL;
	return buffer+(slot_index*GLOBALS::blk_size);
}

bool LRUBuffer::contains(b_id page_id)
{
	if (page_lookup->find(page_id) == page_lookup->end())
		return false;
	return true;
}

bool LRUBuffer::slot_occupied(int slot_index)
{
	return buf_lookup->find(slot_index) != buf_lookup->end();
}

b_id LRUBuffer::get_page_id(int slot_index)
{
	b_id page_id = -1;
	if (buf_lookup->find(slot_index) != buf_lookup->end())
		page_id = (*buf_lookup)[slot_index];
	return page_id;
}
