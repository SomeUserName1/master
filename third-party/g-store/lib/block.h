#ifndef BLOCK_H
#define BLOCK_H

#include "defs.h"

void blk_init_fkts();
g_id get_first_gid(b_id bid);
b_id get_block_id(g_id gid);
b_sc get_slot(g_id gid);
void ie_1_to_str_adv(char* &str, char* b_ptr);
void ie_2_to_str_adv(char* &str, char* b_ptr);
void ee_to_str_adv  (char* &str, char* b_ptr);
void set_ie_1(char* b_ptr, b_sc ie);
void set_ie_2(char* b_ptr, b_sc ie);
void set_ee  (char* b_ptr, g_id ee);
void set_ie_1_adv(char* &b_ptr, b_sc ie);
void set_ie_2_adv(char* &b_ptr, b_sc ie);
void set_ee_adv  (char* &b_ptr, g_id ee);
b_sc get_ie_1(char* b_ptr);
b_sc get_ie_2(char* b_ptr);
g_id get_ee  (char* b_ptr);
void adv_ie_1(char* &b_ptr);
void adv_ie_2(char* &b_ptr);
void adv_ee  (char* &b_ptr);
b_sc get_ie_1_adv(char* &b_ptr);
b_sc get_ie_2_adv(char* &b_ptr);
g_id get_ee_adv  (char* &b_ptr);
char* ee_bs (g_id key, char* ee_begin, char* ee_end);
char* ie_1_bs(b_sc key, char* ie_begin, char* ie_end);
char* ie_2_bs(b_sc key, char* ie_begin, char* ie_end);
uint4 get_header2(header1 h);
uint4 get_header3(header1 h);
void set_header2(header1 h, uint4 i);
void set_header3(header1 h, uint4 i);
void inc_header2(header1 h, uint4 i);
void inc_header3(header1 h, uint4 i);
header1 blk_get_header(block1 b, b_sc slt);
void blk_initialize(block1 b, g_id first_v);
void blk_insert(block1 b, b_sc slt, header1 header, char* fields);
g_id blk_get_first_gid(block1 b);
b_sc blk_get_rec_in_block(block1 b);
b_sc blk_get_max_slt(block1 b);
uint4 blk_get_var_start(block1 b);
void blk_inc_var_start(block1 b, uint4 len);
void blk_inc_rec_num(block1 b);
void blk_extract_fld(block1 b, int fld, int slt, char* &dest, int &len);
void blk_print(block1 b);

#endif