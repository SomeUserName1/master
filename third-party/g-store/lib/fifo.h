#ifndef FIFO_H
#define FIFO_H

#include <assert.h>
#include <list>
#include <algorithm>

class FIFO
{
public:

	FIFO();
	~FIFO(void);

	bool insert(int page);
	void erase(int page);

	int next(int* space, int k, bool read_ahead);

	int get_size();
	bool contains(int page);
	bool is_empty();

private:
	std::list< int >* fifo;
};

#endif