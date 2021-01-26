#ifndef MEMORY_MGR_H
#define MEMORY_MGR_H

#include "defs.h"

void mem_create_disks_blks(int num_blocks, int8 max_disk_size_b = ((int8) 1 << 32)-1);;
void mem_create_disks_g(double total_space_g, double max_disk_size_g = 4.0);
void mem_create_disks_m(double total_space_m, double max_disk_size_m = 4096.0);
void mem_create_disks(int8 space_b, int8 max_disk_size_b = ((int8) 1 << 32)-1);
void mem_reinitialize();
void mem_write_blocks(block1 b, b_id bid, int num);
void mem_write_block(block1 b, b_id bid);
/**
 * @param b, pointer into buffer memory area
 * @param bid, the starting page id to read
 * @param num, the number of pages to read
 */
void mem_read_blocks(block1 b, b_id bid, int num);
void mem_read_block(block1 b, b_id bid);
void mem_print_disks_info();
void mem_close_disks();
void mem_delete_disks();

#endif