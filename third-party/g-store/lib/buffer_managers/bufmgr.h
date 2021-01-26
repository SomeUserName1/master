#ifndef BUFMGR_H
#define BUFMGR_H

#include "defs.h"
#include "structs.h"

/* Header for bufmgr1.cpp, bufmgr2.cpp, bufmgr3.cpp. 
bv_list_slt_* are structures used in the query engine, e.g., to hold 
a block and slot number of a vertext that is not currently in the 
buffer but should be visited later */

//sizeof =  12
struct bv_list_slt_start_with
{
  b_sc slt;
  uint4 start_with_v_num;
  bv_list_slt_start_with* next;
  bv_list_slt_start_with(b_sc slt_, uint4 start_with_v_num_) 
    : slt(slt_), start_with_v_num(start_with_v_num_), next(NULL) {};
  bv_list_slt_start_with() {};
};

//sizeof =  8
struct bv_list_slt
{
  b_sc slt;
  bv_list_slt* next;
  bv_list_slt(b_sc slt_) : slt(slt_), next(NULL) {};
  bv_list_slt(){};
};

//sizeof =  8
struct bv_list_slt_seq
{
  b_sc slt;
  uint2 seq;
  bv_list_slt_seq* next;
  bv_list_slt_seq(b_sc slt_, uint2 seq_) : slt(slt_), seq(seq_), next(NULL) {};
  bv_list_slt_seq(){};
};

template <typename T_>
struct bv_list_begin
{
  T_* begin_v;
  bv_list_begin() : begin_v(NULL) {};
  void add(T_* ptr)
  {
    ptr->next = begin_v;
    begin_v = ptr;
  }
};

struct bufmgr1
{
  bufmgr1(int num_blks, bool use_map);
  ~bufmgr1();
  int num_blks;
  int next; // next page slot to access
  block1 mem;
  block1* mem_blks;
  b_id* blk_bid_map;
  int* bid_blk_map;
  bool use_map;
  //std::hash_map<b_id, uint4> bid_blk_map;
  //TODO rename mem blk to mem slt;

  block1 last_blk;
  b_id* bid_mem_map;
  uint4* mem_bid_map;
  bool run0;
  
  void get_next_blk(block1 &b);
  void fill_cons_from_start_no_map(block1 &b, b_id bid_start, int &num);
  void fill_cons_from_start(block1 &b, b_id bid_start, int &num);
  void test_integrity1();

  template <typename T_>
  bool fill_non_cons_bv_list(block1 &b, bv_list_begin<T_>* pbvlb, int &num);

  bool in_mem(block1 &b, b_id bid);
  void get_block_w(block1 &b, b_id bid, bool is_new);
  void get_block(block1 &b, b_id bid);

  void inc_next();
  void write_memory();
};

//////////////////////////////////////////////////////////////////////////

struct bufmgr2 : public bufmgr1
{
  void* idq_space;
  indexed_dequeue idq;

  bufmgr2(int num_blks);
  ~bufmgr2();
  int get_mem_slot_num(b_id bid);
  void start_traverse(b_id bid);
  bool confirm_block(b_id bid, int &assumed_mem_slt, block1 &b, header1 &h, char* &ptr_in_b);
  void in_mem_or_add_pin(block1 &b, b_id bid);
  void pin(int mem_slt);
  void unpin(int assumed_mem_slt);
  void test_integrity();

  //Overwrite
  void find_or_fill(block1 &b, b_id bid);
};

//////////////////////////////////////////////////////////////////////////

template <typename T_>
struct bufmgr3
{
  struct bid_slt_pair
  {
    b_id bid;
    b_sc slt;
    bid_slt_pair (b_id bid_, b_sc slt_) :  bid(bid_), slt(slt_) {};
    bid_slt_pair(){};
  };

  T_* free_list_start;
  uint4 part_size;
  std::vector<T_*> mem_parts;
  bufmgr3(uint4 part_size_);
  
  ~bufmgr3();
  T_* put(T_ e);
  void give_back(T_* ptr); 
  void add_new_mem_part(); 

  // recycle memory during the printing of the path
  int ref_idx;
  uint ref_part;
  int ref_part_size;
  bid_slt_pair* ref_part_ptr;
  void ref_add(b_id bid, b_sc slt);
  bool ref_get(b_id &bid, b_sc &slt);
  
  bool identify_path_cycle(char* ie_list_begin, char* ie_list_end, char* ee_list_end, int lvl);
};

#endif