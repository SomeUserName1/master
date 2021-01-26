#include "page_queue.h"
#include <assert.h>

bool* pages;
int capacity;
int size;
PageQueue::Direction current_dir;
int last_retrieved_page;

int fill_ascending(int* space, int k, int& fill_ptr, int& gaps);
int fill_descending(int* space, int k, int& fill_ptr, int& gaps);

PageQueue::PageQueue(int cap)
{
	current_dir = ASCENDING;
	last_retrieved_page = 0;
	size = 0;
	capacity = cap;
	pages = new bool [capacity];
	// initialise queue to empty <=> all slots false
	for (int i = 0; i < capacity; ++i) {
		pages[i] = false;
	}
}


PageQueue::~PageQueue(void)
{
	delete pages;
}

bool PageQueue::insert(int page)
{
	// check for out of bounds
	int last_page_id = capacity-1;
	if (page > last_page_id)
		return false;
	
	if (pages[page] == false) {
		pages[page] = true;
		++size;
	}

	return true;
}

bool PageQueue::next(int* space, int k)
{
	if (k > capacity)
		return false;

	int num_filled = 0;
	int fill_ptr = 0;
	bool not_enough_pages = (size < k) ? true : false;

	int first_page = last_retrieved_page;
	int gaps = 0;
		
	if (not_enough_pages)
		gaps = k - size;

	if (current_dir == ASCENDING) {
		num_filled += fill_ascending(space, k, fill_ptr, gaps);

		if (num_filled < k) {
			current_dir = DESCENDING;
			if (not_enough_pages)
				last_retrieved_page = first_page;
			num_filled += fill_descending(space, k, fill_ptr, gaps);
		}
	} else { // current_dir == DESCENDING
		num_filled += fill_descending(space, k, fill_ptr, gaps);

		if (num_filled < k) {
			current_dir = ASCENDING;
			if (not_enough_pages)
				last_retrieved_page = first_page;
			num_filled += fill_ascending(space, k, fill_ptr, gaps);
		}
	}

	assert (gaps == 0);
	if (not_enough_pages)
		assert (size == 0);
	assert (num_filled == k);

	// make sure direction is correct if at ends of array
	if (last_retrieved_page == 0)
		current_dir = ASCENDING;
	else if (last_retrieved_page == (capacity-1))
		current_dir = DESCENDING;

	return true;
}

int PageQueue::get_size()
{
	return size;
}

int PageQueue::get_capacity()
{
	return capacity;
}

int PageQueue::get_last_retrieved_page()
{
	return last_retrieved_page;
}

PageQueue::Direction PageQueue::get_current_dir()
{
	return current_dir;
}

bool PageQueue::contains(int page)
{
	return pages[page];
}

bool PageQueue::is_empty()
{
	return get_size() == 0;
}

int fill_ascending(int* space, int k, int& fill_ptr, int& gaps) {
	assert (current_dir == PageQueue::ASCENDING);

	int num_filled = 0;

	for (int i = last_retrieved_page; i < capacity && num_filled < k; ++i) {
		if (pages[i] == true) {
			space[fill_ptr] = i;
			pages[i] = false; // remove from page queue
			--size;
			++fill_ptr;
			++num_filled;
			last_retrieved_page = i;
		} else if (gaps > 0) {
			space[fill_ptr] = i;
			++fill_ptr;
			++num_filled;
			--gaps;
			last_retrieved_page = i;
		}
	}

	return num_filled;
}

int fill_descending(int* space, int k, int& fill_ptr, int& gaps) {
	assert (current_dir == PageQueue::DESCENDING);

	int num_filled = 0;

	for (int i = last_retrieved_page; i >= 0 && num_filled < k; --i) {
		if (pages[i] == true) {
			space[fill_ptr] = i;
			pages[i] = false; // remove from page queue
			--size;
			++fill_ptr;
			++num_filled;
			last_retrieved_page = i;
		} else if (gaps > 0) {
			space[fill_ptr] = i;
			++fill_ptr;
			++num_filled;
			--gaps;
			last_retrieved_page = i;
		}
	}

	return num_filled;
}