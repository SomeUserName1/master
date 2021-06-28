#ifndef REORGANIZE_RECORDS_H
#define REORGANIZE_RECORDS_H

#include "query/operators.h"

unsigned long*
remap_node_ids(in_memory_file_t* db, const unsigned long* partition);

#endif
