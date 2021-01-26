#ifndef READ_TO_BLOCKS_H
#define READ_TO_BLOCKS_H

#include "defs.h"

void read_to_blocks(FILE* fp_in, FILE* fp_gid_map);
void parse_fields(g_id me, b_id my_bid, char* str0, char* fields, header1 header, uint4 var_start, g_id* gid_map);

#endif