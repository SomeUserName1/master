#ifndef DEGREE_H
#define DEGREE_H
#include "access/operators.h"
#include <stdio.h>

size_t
get_degree(in_memory_file_t* db,
           unsigned long     node_id,
           direction_t       direction,
           FILE*             log_file);

float
get_avg_degree(in_memory_file_t* db, direction_t direction, FILE* log_file);

size_t
get_min_degree(in_memory_file_t* db, direction_t direction);

size_t
get_max_degree(in_memory_file_t* db, direction_t direction);

#endif
