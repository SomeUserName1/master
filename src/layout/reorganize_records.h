#ifndef REORGANIZE_RECORDS
#define REORGANIZE_RECORDS

#include "../access/in_memory_file.h"

unsigned long*
remap_node_ids(in_memory_file_t* db, unsigned long* partition);

unsigned long*
remap_rel_ids(in_memory_file_t* db);

void
sort_incidence_list(in_memory_file_t* db);

void
reorganize_records(in_memory_file_t* db, unsigned long* graph_partition);

#endif
