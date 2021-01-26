#ifndef PAGEQUEUE_H
#define PAGEQUEUE_H

typedef enum dir : unsigned char
{
	ASCENDING,
	DESCENDING
} Direction;

class PageQueueImpl;

class PageQueue
{
public:
	PageQueue(int capacity = 100);
	~PageQueue(void);

    /**
	 * Inserts the given page.
	 * @param page, the page identifier
	 * @return false if invalid page identifier
	 */
	bool insert(int page);
	void erase(int page);

	/**
	 * Retrieves k of the next pages in the queue.
	 * If k > size and read_ahead=1, any valid page id 
	 * may be included in order to return k page ids.
	 * @param space, allocated space for up to k page ids
	 * @param k, the number of requested pages
	 * @param read_ahead, 1: performs read-ahead of pages
	 * @return number of retrieved pages
	 */
	int next(int* space, int k, bool read_ahead);

	int get_size();
	int get_capacity();
	int get_last_retrieved_page();
	bool contains(int page);
	bool is_empty();
	Direction get_current_dir();

private:
	PageQueueImpl* pimpl;
};

#endif