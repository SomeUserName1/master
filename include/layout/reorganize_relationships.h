#ifndef REORGANIZE_RELATIONSHIPS_H
#define REORGANIZE_RELATIONSHIPS_H

#include "access/operators.h"

unsigned long*
remap_rel_ids(in_memory_file_t* db);

void
sort_incidence_list(in_memory_file_t* db);

#endif
