#ifndef QUERY_SHARED_H
#define QUERY_SHARED_H

#include "defs.h"
#include "buffer_managers/bufmgr.h"

void query_vertex();
void print_query_header();
template <typename T_>
void print_found_path(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<T_>* bm3, g_id* p_v_offset, g_id* v_found_paths);
void print_found_path_seq(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<bv_list_slt_seq>* bm3, g_id* p_v_offset, g_id** v_found_paths, int seq);
template <typename T_>
void print_found_path_common(bufmgr1* bm1, bufmgr3<T_>* bm3);
void print_query_rec_new(block1 b, header1 h, uint4 recnum, g_id gid, int lvl, int &sel_path_len, bool iscycle);
void add_to_sel_root_arr(block1 b, header1 h);
void add_to_sel_path(block1 b, header1 h, int &sel_path_len);
void push_on_output(char* str1);
void register_finished();

#endif