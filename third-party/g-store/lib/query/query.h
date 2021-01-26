#ifndef QUERY_H
#define QUERY_H

#include "defs.h"
#include "buffer_managers/bufmgr.h"
#include "util_t.h"
#include "util.h"
#include "block.h"
#include "structs.h"
#include "query_shared.h"
#include "read_query.h"
#include "pred_tree.h"
#include "parameters.h"
#include "page_queue.h"
#include "buffer_managers/lru.h"
#include <unordered_map>
#include <stack>
#include <queue>

struct query 
{
  virtual void start() = 0;
  virtual ~query() {} 
};

struct q_vertex : public query
{
  bufmgr1 bm1;
  q_vertex();
  ~q_vertex();

  void start();
};


struct q_trav : public query
{
  g_id* trav_list;
  //b_sc* cycle_list;

  bufmgr2 bm2;

  void start();
  bool traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, uint4 &row_num, int lvl, int sel_path_len);
  //void identify_cycles(char* ie_list_begin, char* ie_list_end, char* ee_list_end, b_id my_bid, int lvl, b_sc &num_cycles);
  
  q_trav();
  ~q_trav();
};


// force enums to be chars, thus saving memory
typedef enum enum_state : unsigned char
{
	//Undiscovered,
	Discovered
} State;

// force enums to be chars, thus saving memory
typedef enum enum_label : unsigned char
{
	//Unlabeled,
	Source,
	Destination,
	Through,
	Unfit
} Label;

void evaluate_label(block1 p, header1 h, g_id first_gid, int v_rel, 
		std::unordered_map< unsigned int, Label > &label, g_id v_gid);

class q_path_set : public query
{
public:
	void start();
	PageQueue *get_page_queue() {
		return page_queue;
	};

	q_path_set();
	~q_path_set();

	void print_stats();

private:
	LRUBuffer *lru;
	std::unordered_map< unsigned int, State > state;
	std::unordered_map< unsigned int, Label > label;
	//State* state;
	//Label* label;
	PageQueue *page_queue;
	std::unordered_map< unsigned int, std::stack< g_id >* >* stacks;

	int tot_reads;
	int page_reqs;
	int cache_hits;
	int* page_stats;
	int mult_pages;
};

class q_spath_set : public query
{
public:

	void start();

	q_spath_set();
	~q_spath_set();

	void print_stats();

private:
	LRUBuffer* lru;
	std::unordered_map< unsigned int, State > state;
	std::unordered_map< unsigned int, Label > label;
	std::unordered_map< unsigned int, unsigned int > parent;
	PageQueue *page_queue_curr;
	PageQueue *page_queue_next;
	PageQueue **p_pq_curr;
	PageQueue **p_pq_next;

	/*
	 * Maps page_id -> vertex queue for that page.
	 * Enqueued elements are relative identifiers within the page
	 */
	std::unordered_map< unsigned int, std::queue< int >* > *v_queue_curr;
	std::unordered_map< unsigned int, std::queue< int >* > *v_queue_next;
	std::unordered_map< unsigned int, std::queue< int >* > **p_vq_curr;
	std::unordered_map< unsigned int, std::queue< int >* > **p_vq_next;

	int tot_reads;
	int page_reqs;
	int cache_hits;
	int* page_stats;
	int mult_pages;
};

struct q_path_node : public query
{
  bv_list_begin<bv_list_slt>* p_bv_list_begin;
  g_id* p_v_offset;
  g_id* v_found_paths;

  bufmgr1 bm1;
  bufmgr3<bv_list_slt> bm3;

  void start();
  bool traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0, int lvl);

  q_path_node();
  ~q_path_node();
};

struct q_path_seq : public query
{
  bv_list_begin<bv_list_slt_seq>* p_bv_list_begin;
  g_id* p_v_offset;
  g_id* v_found_paths_;
  g_id** v_found_paths;

  bufmgr1 bm1;
  bufmgr3<bv_list_slt_seq> bm3;

  void start();
  void mark_all(b_id bid, b_sc slt, g_id mark_gid);
  bool traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0, int lvl, int seq);

  q_path_seq();
  ~q_path_seq();
};


struct q_spath : public query
{
  bv_list_begin<bv_list_slt>* p_bv_list_begin_cur;
  bv_list_begin<bv_list_slt>* p_bv_list_begin_nxt;
  g_id* p_v_offset;
  g_id* v_found_paths;

  bufmgr1 bm1;
  bufmgr3<bv_list_slt> bm3;

  void start();
  bool traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0);

  q_spath();
  ~q_spath();
};

struct q_spath_tree  : public query
{
  struct str_len_pair
  {
    char* str;
    int len;
    str_len_pair(char* str_, int len_) : str(str_), len(len_) {};
  };

  bv_list_begin<bv_list_slt_start_with>* p_bv_list_begin_cur;
  bv_list_begin<bv_list_slt_start_with>* p_bv_list_begin_nxt;
  g_id* p_v_offset;
  g_id* v_found_paths;

  std::vector<str_len_pair> sel_paths;
  std::vector<char**> sel_root_arrs;

  bufmgr1 bm1;
  bufmgr3<bv_list_slt_start_with> bm3;
  q_spath_tree();
  ~q_spath_tree();

  void start();
  void traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0, uint4 my_start_v_cnt);

  void print_query_rec_new_wrap(block1 b, header1 h, b_id my_bid, b_sc my_slt, g_id my_gid, int lvl, uint4 my_start_v);
  void handle_start_with_v(block1 b, header1 h);

  uint4 row_num;
  uint4 cnt_start_v;
};

#endif