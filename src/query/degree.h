#ifndef DEGREE
#define DEGREE
#include "../access/in_memory_file.h"

float get_avg_degree(in_memory_file_t* db, direction_t direction);
size_t get_min_degree(in_memory_file_t* db, direction_t direction);
size_t get_max_degree(in_memory_file_t* db, direction_t direction);

#endif
