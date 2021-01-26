#include "page_queue.h"
#include <assert.h>

class PageQueueImpl
{
public:

	PageQueueImpl(int capacity = 100);
	~PageQueueImpl(void);

	bool insert(int page);
	void erase(int page);

	int next(int* space, int k, bool read_ahead);

	int get_size();
	int get_capacity();
	int get_last_retrieved_page();
	bool contains(int page);
	bool is_empty();
	Direction get_current_dir();

private:
	bool* pages;
	int capacity;
	int size;
	int last_retrieved_page; // <=> head

	int fill_ascending(int* space, int k, int& fill_ptr, int& gaps);
};

PageQueue::PageQueue(int cap)
{
	pimpl = new PageQueueImpl(cap);
}

PageQueue::~PageQueue(void)
{
	delete pimpl;
}

bool PageQueue::insert(int page)
{
	return pimpl->insert(page);
}

void PageQueue::erase(int page)
{
	return pimpl->erase(page);
}

int PageQueue::next(int* space, int k, bool read_ahead)
{
	return pimpl->next(space, k, read_ahead);
}

int PageQueue::get_size()
{
	return pimpl->get_size();
}

int PageQueue::get_capacity()
{
	return pimpl->get_capacity();
}

int PageQueue::get_last_retrieved_page()
{
	return pimpl->get_last_retrieved_page();
}

Direction PageQueue::get_current_dir()
{
	return pimpl->get_current_dir();
}

bool PageQueue::contains(int page)
{
	return pimpl->contains(page);
}

bool PageQueue::is_empty()
{
	return pimpl->is_empty();
}

/*
 * Implementation
 */
PageQueueImpl::PageQueueImpl(int cap)
{
	last_retrieved_page = 0;
	size = 0;
	capacity = cap;
	pages = new bool [capacity];
	// initialise queue to empty <=> all slots false
	for (int i = 0; i < capacity; ++i) {
		pages[i] = false;
	}
}

PageQueueImpl::~PageQueueImpl(void)
{
	delete pages;
}

bool PageQueueImpl::insert(int page)
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

void PageQueueImpl::erase(int page)
{
	if (pages[page] == true) {
		pages[page] = false;
		--size;
	}
}

int PageQueueImpl::next(int* space, const int k, bool read_ahead)
{
	if (k > capacity)
		return false;

	int num_filled = 0;
	int fill_ptr = 0;
	bool not_enough_pages = (size < k) ? true : false;

	int gaps = 0;
		
	// if k > size, and read_ahead true, read gaps
	if (read_ahead && not_enough_pages)
		gaps = k - size;

	num_filled += fill_ascending(space, k, fill_ptr, gaps);

	if (num_filled < k) {
		last_retrieved_page = 0;
		num_filled += fill_ascending(space, (k-num_filled), fill_ptr, gaps);
	}

	//assert (gaps == 0);
	//if (not_enough_pages)
	//	assert (size == 0);
	//assert (num_filled == k);

	return num_filled;
}

int PageQueueImpl::get_size()
{
	return size;
}

int PageQueueImpl::get_capacity()
{
	return capacity;
}

int PageQueueImpl::get_last_retrieved_page()
{
	return last_retrieved_page;
}

Direction PageQueueImpl::get_current_dir()
{
	return ASCENDING;
}

bool PageQueueImpl::contains(int page)
{
	return pages[page];
}

bool PageQueueImpl::is_empty()
{
	return get_size() == 0;
}

int PageQueueImpl::fill_ascending(int* space, const int k, int& fill_ptr, int& gaps) {
	//assert (current_dir == PageQueue::ASCENDING);

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