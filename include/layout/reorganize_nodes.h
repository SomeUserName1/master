#ifndef REORGANIZE_RECORDS_H
#define REORGANIZE_RECORDS_H

#include "access/heap_file.h"

unsigned long*
remap_node_ids(heap_file* hf, const unsigned long* partition);

#endif
