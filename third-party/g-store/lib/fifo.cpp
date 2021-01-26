#include "fifo.h"

/*
 * Implementation
 */
FIFO::FIFO()
{
	fifo = new std::list< int >();
}

FIFO::~FIFO(void)
{
	delete fifo;
}

bool FIFO::insert(int page)
{
	std::list< int >::iterator it = std::find(fifo->begin(), fifo->end(), page);
	if (it == fifo->end()) {
		fifo->push_back(page);
	}

	return true;
}

void FIFO::erase(int page)
{
	std::list< int >::iterator it = std::find(fifo->begin(), fifo->end(), page);
	if (it != fifo->end()) {
		fifo->erase(it);
	}
}

int FIFO::next(int* space, const int k, bool read_ahead)
{
	int num_fill = (fifo->size() > (unsigned int) k) ? k : fifo->size();

	for (int i = 0; i < num_fill; ++i) {
		space[i] = fifo->front();
		fifo->pop_front();
	}

	return num_fill;
}

int FIFO::get_size()
{
	return fifo->size();
}

bool FIFO::contains(int page)
{
	std::list< int >::iterator it = std::find(fifo->begin(), fifo->end(), page);

	return it != fifo->end();
}

bool FIFO::is_empty()
{
	return get_size() == 0;
}
