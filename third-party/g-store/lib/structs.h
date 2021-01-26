#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <vector>

#include "defs.h"

/* Custom structs used in various algorithms */

//////////////////////////////////////////////////////////////////////////

struct graph1
{
  int num_v, num_e, num_p, c_level;
  int* offset_e;  //size: num_v
  int* v_w;       //size: num_v
  int* map_in_c;  //size: num_v
  int* e_to;      //size: num_e
  int* e_w;       //size: num_e
  int* part;      //size: num_v
  int* p_w;       //size: num_p
  FILE* on_disk;
  graph1 *coarser, *finer;
  
  void init()
  {
    num_v = num_e = num_p = c_level = -1;
    offset_e = v_w = map_in_c = e_to = e_w = part = p_w = NULL;
    on_disk = NULL;
    coarser = finer = NULL;
  };
};

//////////////////////////////////////////////////////////////////////////

template <typename T_>
struct idx_sc_pair 
{
  int idx;
  T_ sc;

  void set(int idx_, T_ sc_)
  {
    idx = idx_;
    sc = sc_;
  }

  void if_sm_set(int idx_, T_ sc_)
  {
    if (sc_ < sc)
      set(idx_, sc_);
  }

  void if_gr_set(int idx_, T_ sc_)
  {
    if (sc_ > sc)
      set(idx_, sc_);
  }
};

struct idx_scd_w_pair 
{
  int idx;
  int w;
  double sc;
};

//////////////////////////////////////////////////////////////////////////

struct priority_queue1
{
  int cnt_elem; 
  idx_sc_pair<int>* pq; 
  int* pos;

  priority_queue1(){};

  priority_queue1(void* pq_space, int size)
  { 
    init(pq_space, size);
  };

  void init(void* pq_space, int size)
  {
    pq = (idx_sc_pair<int>*) pq_space;
    pos = (int*) (pq + size + 1);
    cnt_elem = 0; 
  };

  void insert(int idx, int sc)
  { 
    pq[++cnt_elem].idx = idx; 
    pq[cnt_elem].sc = sc; 
    pos[idx] = cnt_elem; 
    heap_up(cnt_elem); 
  }

  idx_sc_pair<int> pop_max()
  {
    swap1(pq[1], pq[cnt_elem]);
    heap_down(1, cnt_elem-1);
    return pq[cnt_elem--];
  }

  void change(int idx, int sc)
  { 
    pq[pos[idx]].sc = sc;
    heap_up(pos[idx]); 
    heap_down(pos[idx], cnt_elem); 
  }

  bool empty()
  { 
    return cnt_elem == 0;
  }

  void reset()
  { 
    cnt_elem = 0;
  }

private:
  void heap_up(int k)
  {
    while (k > 1 && pq[k/2].sc < pq[k].sc)
    { 
      swap1(pq[k], pq[k/2]); 
      k = k/2; 
    }
  }

  void heap_down(int k, int n)
  {
    int j;
    while (2*k <= n)
    { 
      j = 2*k;
      if (j < n && pq[j].sc < pq[j+1].sc) 
        j++;
      if (pq[k].sc >= pq[j].sc) 
        break;
      swap1(pq[k], pq[j]); 
      k = j;
    }
  }

  void swap1(idx_sc_pair<int> i, idx_sc_pair<int> j)
  { 
    int t = pos[i.idx]; 
    pos[i.idx] = pos[j.idx]; 
    pos[j.idx] = t;
    pq[pos[i.idx]] = i; 
    pq[pos[j.idx]] = j;
  }
};


//////////////////////////////////////////////////////////////////////////

struct idx_p_sc
{
  int idx;
  int p;
  double sc;
};

struct priority_queue2
{
  int cnt_elem, cnt_elem_r; 
  int* pq;
  idx_p_sc* pos;
  int size;

  priority_queue2(){};

  priority_queue2(void* pq_space, int size_)
  { 
    init(pq_space, size_);
  };

  void init(void* pq_space, int size_)
  {
    size = size_;
    pq = (int*) pq_space;
    pos = (idx_p_sc*) (pq + size + 1);
    cnt_elem = 0; 
    cnt_elem_r = size + 1; 
  };

  void insert(int v, int p, double sc)
  { 
    pq[++cnt_elem] = v; 
    pos[v].sc = sc; 
    pos[v].p = p;
    pos[v].idx = cnt_elem; 
    heap_up(cnt_elem); 
  }

  void insert_r(int v)
  { 
    pos[v].idx = --cnt_elem_r;  
    pq[cnt_elem_r] = v;
  }

  bool test_pq()
  {
    int i;
    for (i = 1; i<=cnt_elem;i++)
      if (pos[pq[i]].idx!= i)
        return false;
    for (i = cnt_elem_r; i<size;i++)
      if (pos[pq[i]].idx!= i)
        return false;
    return true;
  }
  void remove_r(int v)
  { 
    if (v != pq[cnt_elem_r])
    {
      pos[pq[cnt_elem_r]].idx = pos[v].idx;
      pq[pos[v].idx] = pq[cnt_elem_r];
    }
    cnt_elem_r++;
    //pos[v].idx to be changed outside
  }

  void pop_max(int &v, int &p, double &sc)
  {
    swap1(pq[1], pq[cnt_elem]);
    heap_down(1, cnt_elem-1);
    v = pq[cnt_elem--];
    p = pos[v].p;
    sc = pos[v].sc;
  }

  void change(int v, int p, double sc)
  {
    assert(pos[v].idx <= cnt_elem);
    pos[v].sc = sc;
    pos[v].p = p;
    heap_up(pos[v].idx); 
    heap_down(pos[v].idx, cnt_elem); 
  }

  bool empty()
  { 
    return cnt_elem == 0;
  }

  void reset()
  { 
    cnt_elem = 0;
    cnt_elem_r = size + 1;
  }
  
  void change_if_better(int v, int p, double sc)
  { 
    if (sc > pos[v].sc)
      change(v, p, sc);
  }

//private:
  void heap_up(int k)
  {
    while (k > 1 && pos[pq[k/2]].sc < pos[pq[k]].sc)
    { 
      swap1(pq[k], pq[k/2]); 
      k = k/2; 
    }
  }

  void heap_down(int k, int n)
  {
    int j;
    while (2*k <= n)
    { 
      j = 2*k;
      if (j < n && pos[pq[j]].sc < pos[pq[j+1]].sc) 
        j++;
      if (pos[pq[k]].sc >= pos[pq[j]].sc) 
        break;
      swap1(pq[k], pq[j]); 
      k = j;
    }
  }

  void swap1(int i, int j)
  { 
    int t = pos[i].idx; 
    pos[i].idx = pos[j].idx; 
    pos[j].idx = t;
    pq[pos[i].idx] = i; 
    pq[pos[j].idx] = j;
  }
};


//////////////////////////////////////////////////////////////////////////
// used in project.cpp

struct extemes
{
  int min_idx;
  int min_tension;
  int max_idx;
  int max_tension;  
  int global_min_idx;
  int global_min_tension;
  int global_max_idx;
  int global_max_tension;

  extemes() 
  {
    reset();
  }

  void reset(){
    min_idx = -1;
    min_tension = INT_MAX;
    max_idx = -1;
    max_tension = INT_MIN;
    global_min_idx = -1;
    global_min_tension = INT_MAX;
    global_max_idx = -1;
    global_max_tension = INT_MIN;

  }

  void update(int idx, int tension, int indicator){
    // part[ ] >= -1  ->  not connected to current partition or already sorted
    // part[ ] == -2  ->  connected to current partition, not connected to left or right BF-tree
    // part[ ] == -3  ->  connected to current partition, connected to left BF-tree
    // part[ ] == -4  ->  connected to current partition, connected to right BF-tree
    // part[ ] == -5  ->  connected to current partition, connected to left and right BF-tree
    switch(indicator){
    case -5:
      if (tension < min_tension){
        min_tension = tension;
        min_idx = idx;
      }
      if (tension > max_tension){
        max_tension = tension;
        max_idx = idx;
      }
      break;
    case -3:
      if (tension < min_tension){
        min_tension = tension;
        min_idx = idx;
      }
      break;
    case -4:
      if (tension > max_tension){
        max_tension = tension;
        max_idx = idx;
      }
      break;
    }

    if (min_idx==-1 && indicator<=-2)
      if (tension < global_min_tension){
        global_min_tension = tension;
        global_min_idx = idx;
      }

      if (max_idx==-1 && indicator<=-2)
        if (tension > global_max_tension){
          global_max_tension = tension;
          global_max_idx = idx;
        }
  }
  bool done()
  {
    if (max_idx == -1)
      max_idx = global_max_idx;
    if (min_idx == -1)
      min_idx = global_min_idx;

    return (min_idx == -1);
  }
};

//////////////////////////////////////////////////////////////////////////
// used in evaluate.cpp

struct p_stats
{
  int cnt_num_v_p;
  int cnt_int_p;
  int cnt_ext_p; 
  int8 cnt_tens_p;
  idx_sc_pair<int> min_int_v;
  idx_sc_pair<int> max_int_v; 
  idx_sc_pair<int> min_ext_v;
  idx_sc_pair<int> max_ext_v; 
  idx_sc_pair<int8> min_tens_v;
  idx_sc_pair<int8> max_tens_v;

  void reset()
  {
    cnt_int_p = cnt_ext_p = cnt_num_v_p = 0;
    cnt_tens_p = 0L;
    min_int_v.set(-1, INT_MAX);
    max_int_v.set(-1, -1);
    min_ext_v.set(-1, INT_MAX);
    max_ext_v.set(-1, -1);
    min_tens_v.set(-1, INT8_MAX);
    max_tens_v.set(-1, -1L);
  }

  void update(int idx, int cnt_int, int cnt_ext, int8 cnt_tens)
  {
    cnt_num_v_p++;
    cnt_int_p += cnt_int;
    cnt_ext_p += cnt_ext;
    cnt_tens_p += cnt_tens;

    min_int_v.if_sm_set(idx, cnt_int);
    max_int_v.if_gr_set(idx, cnt_int);
    min_ext_v.if_sm_set(idx, cnt_ext);
    max_ext_v.if_gr_set(idx, cnt_ext);   
    min_tens_v.if_sm_set(idx, cnt_tens);
    max_tens_v.if_gr_set(idx, cnt_tens);
  }
};

//////////////////////////////////////////////////////////////////////////
// used in evaluate.cpp

struct g_stats
{
  int cnt_num_v_g;
  int cnt_num_p_g;
  int8 cnt_int_g;
  int8 cnt_ext_g; 
  int8 cnt_tens_g;
  int8 cnt_p_w;

  idx_sc_pair<int> min_int_p;
  idx_sc_pair<int> max_int_p; 
  idx_sc_pair<int> min_ext_p;
  idx_sc_pair<int> max_ext_p; 
  idx_sc_pair<int8> min_tens_p;
  idx_sc_pair<int8> max_tens_p;

  idx_sc_pair<int> min_w_p;
  idx_sc_pair<int> max_w_p;

  idx_sc_pair<int> min_int_v;
  idx_sc_pair<int> max_int_v; 
  idx_sc_pair<int> min_ext_v;
  idx_sc_pair<int> max_ext_v; 
  idx_sc_pair<int8> min_tens_v;
  idx_sc_pair<int8> max_tens_v;

  void init()
  {
    cnt_num_v_g = cnt_num_p_g = 0;
    cnt_int_g = cnt_ext_g = cnt_tens_g = cnt_p_w = 0L;

    min_int_p.set(-1, INT_MAX);
    max_int_p.set(-1, -1);
    min_ext_p.set(-1, INT_MAX);
    max_ext_p.set(-1, -1);
    min_tens_p.set(-1, INT8_MAX);
    max_tens_p.set(-1, -1L);

    min_w_p.set(-1, INT_MAX);
    max_w_p.set(-1, -1);

    min_int_v.set(-1, INT_MAX);
    max_int_v.set(-1, -1);
    min_ext_v.set(-1, INT_MAX);
    max_ext_v.set(-1, -1);
    min_tens_v.set(-1, INT8_MAX);
    max_tens_v.set(-1, -1L);
  }

  void update(int idx, p_stats p_st, int p_w)
  {
    cnt_num_v_g += p_st.cnt_num_v_p;
    cnt_num_p_g++;
    cnt_p_w += p_w;

    cnt_int_g += p_st.cnt_int_p;
    cnt_ext_g += p_st.cnt_ext_p;
    cnt_tens_g += p_st.cnt_tens_p;

    min_int_p.if_sm_set(idx, p_st.cnt_int_p);
    max_int_p.if_gr_set(idx, p_st.cnt_int_p);
    min_ext_p.if_sm_set(idx, p_st.cnt_ext_p);
    max_ext_p.if_gr_set(idx, p_st.cnt_ext_p);   
    min_tens_p.if_sm_set(idx, p_st.cnt_tens_p);
    max_tens_p.if_gr_set(idx, p_st.cnt_tens_p);

    min_w_p.if_sm_set(idx, p_w);
    max_w_p.if_gr_set(idx, p_w);

    min_int_v.if_sm_set(p_st.min_int_v.idx, p_st.min_int_v.sc);
    max_int_v.if_gr_set(p_st.max_int_v.idx, p_st.max_int_v.sc);
    min_ext_v.if_sm_set(p_st.min_ext_v.idx, p_st.min_ext_v.sc);
    max_ext_v.if_gr_set(p_st.max_ext_v.idx, p_st.max_ext_v.sc);   
    min_tens_v.if_sm_set(p_st.min_tens_v.idx, p_st.min_tens_v.sc);
    max_tens_v.if_gr_set(p_st.max_tens_v.idx, p_st.max_tens_v.sc);
  }
};

//////////////////////////////////////////////////////////////////////////
// used in bufmgr

struct indexed_dequeue
{
  struct ideq_elem
  {
    int idx;
    int pin_cnt;
    ideq_elem* left;
    ideq_elem* right;
  };

  ideq_elem* ideq_;
  ideq_elem* ideq;
  ideq_elem** pos;
  ideq_elem* ptr_back;
  ideq_elem* ptr_begin;
  ideq_elem* ptr_last_up;
  int size;
  std::vector<int> elems;
  //comment: cannot know which direction is best.
  //next is first;
  // we do not care about putting first->left and last->right to NULL, will not be used
  indexed_dequeue(){}

  int mem_requirements(int size_);
  void init(void* mem, int size_);
  void reset(int cur_pos);
  void pin_existing(int idx);
  int pin_new();
  void unpin(int idx);
  void print_list_idx();

private:
  void insert_as_back(ideq_elem* tmp);
  void insert_as_begin(ideq_elem* tmp);
  void insert_after(ideq_elem* tmp, ideq_elem* after_what);
  void take_out_begin();
  void take_out_back();
  void take_out_middle(ideq_elem* tmp);
  void do_nothing1();
  
};

//////////////////////////////////////////////////////////////////////////

struct rec_struct
{
  char field_name[64];
  data_types field_datatype;
  int field_length;
  int h_fix_off1;
  int h_var_fld1;
  char sel_char;
  void (*append_to_str)(char* &, char*, int);
  void (rec_struct::*eff_append_to_str)(char*&, block1, header1);

  void eff_append_to_str_num_bool(char* &str0, block1 b, header1 h)
  {
    append_to_str(str0, b + (get_header(h) + h_fix_off1), -1);
  }

  void eff_append_to_str_fixchar(char* &str0, block1 b, header1 h)
  {
    append_to_str(str0, b + (get_header(h) + h_fix_off1), field_length);
  }
  
  void eff_append_to_str_varchar_1(char* &str0, block1 b, header1 h)
  {
    append_to_str(str0, b + (get_header(h) + h_fix_off1), get_header(h + GLOBALS::header_slt_len) - (get_header(h) + h_fix_off1));
  }
  
  void eff_append_to_str_varchar_n(char* &str0, block1 b, header1 h)
  {
    append_to_str(str0, b + get_header(h + h_var_fld1), get_header(h + h_var_fld1 + GLOBALS::header_slt_len) - get_header(h + h_var_fld1));
  }
  
  rec_struct(data_types field_datatype_, char* field_name_) : field_datatype(field_datatype_)
  {
    strcpy(field_name, field_name_);
  };

  rec_struct(){};
};

//////////////////////////////////////////////////////////////////////////

struct one_byte
{ 
  unsigned char _0 : 1;
  unsigned char _1 : 1;
  unsigned char _2 : 1;
  unsigned char _3 : 1;
  unsigned char _4 : 1;
  unsigned char _5 : 1;
  unsigned char _6 : 1;
  unsigned char _7 : 1;
};

//////////////////////////////////////////////////////////////////////////

#endif