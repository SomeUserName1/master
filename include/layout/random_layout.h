#ifndef RANDOM_LAYOUT_H
#define RANDOM_LAYOUT_H

#include "query/operators.h"

unsigned long*
identity_partition(in_memory_file_t* db);

unsigned long*
random_partition(in_memory_file_t* db);

#endif
