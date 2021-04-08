#ifndef TRIVIAL
#define TRIVIAL

#include "../access/in_memory_file.h"

unsigned long*
identity_partition(in_memory_file_t* db);

unsigned long*
random_partition(in_memory_file_t* db);

#endif
