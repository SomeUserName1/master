#ifndef REORGANIZE_RELATIONSHIPS_H
#define REORGANIZE_RELATIONSHIPS_H

#include "access/heap_file.h"

unsigned long*
remap_rel_ids(heap_file* hf);

void
sort_incidence_list(heap_file* hf);

#endif
