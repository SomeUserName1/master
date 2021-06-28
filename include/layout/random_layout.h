#ifndef RANDOM_LAYOUT_H
#define RANDOM_LAYOUT_H

#include "access/in_memory_file.h"

unsigned long*
random_partition(in_memory_file_t* db);

#endif
