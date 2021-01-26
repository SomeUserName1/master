#include "bufmgr.h"

#include "util_t.h"
#include "block.h"

/* Used for q_path, q_path_seq, q_spath, q_spath_tree to hold bv_list_slt_*. 
bufmgr3 allocates memory in chunks ("parts"). An additional chunk is allocated
if the previously allocated chunks are full. */
//#include <g-store.h>

template <typename T_>
bufmgr3<T_>::bufmgr3(uint4 part_size_) : part_size(part_size_)
{
  add_new_mem_part();
  ref_idx = ref_part = 0;
  ref_part_ptr = (bid_slt_pair*) mem_parts.back();
  ref_part_size = part_size * sizeof(T_) / sizeof(bid_slt_pair);
}

template <typename T_>
void bufmgr3<T_>::add_new_mem_part()
{
  T_* tmp = malloc1<T_>(part_size, "todo");
  mem_parts.push_back(tmp);
  free_list_start = tmp;

  for (uint4 i=1; i<part_size; i++)
  {
    tmp->next = (tmp + 1);
    tmp++;
  }

  tmp->next = NULL;
}

template <typename T_>
bufmgr3<T_>::~bufmgr3()
{
  for (std::vector<T_*>::iterator it = mem_parts.begin(); it != mem_parts.end(); it++)
    free(*it);
}

template <typename T_>
T_* bufmgr3<T_>::put(T_ e)
{
  if (free_list_start->next == NULL)
  {
    T_* tmp = free_list_start;
    *tmp = e;
    add_new_mem_part();
    return tmp;
  }
  else
  {
    T_* tmp = free_list_start;
    free_list_start = free_list_start->next;
    *tmp = e;
    return tmp;
  }
}

template <typename T_>
void bufmgr3<T_>::give_back(T_* ptr)
{
  ptr->next = free_list_start;
  free_list_start = ptr;
}

template <typename T_>
void bufmgr3<T_>::ref_add(b_id bid, b_sc slt)
{
  ref_part_ptr[ref_idx++] = bid_slt_pair(bid, slt);
  if (ref_idx == part_size)
  {
    if (++ref_part > mem_parts.size())
      mem_parts.push_back(malloc1<T_>(part_size, "todo"));

    ref_part_ptr = (bid_slt_pair*) mem_parts[ref_part];
    ref_idx = 0;
  }
}

template <typename T_>
bool bufmgr3<T_>::ref_get(b_id &bid, b_sc &slt)
{
  if (ref_idx == 0)
    if (ref_part == 0)
      return false;
    else
    {
      ref_part_ptr = (bid_slt_pair*) mem_parts[--ref_part];
      ref_idx = ref_part_size - 1;
    }
  else
    ref_idx--;

  bid = ref_part_ptr[ref_idx].bid;
  slt = ref_part_ptr[ref_idx].slt;
  return true;
}

template <typename T_>
bool bufmgr3<T_>::identify_path_cycle(char* ie_list_begin, char* ie_list_end, char* ee_list_end, int lvl)
{
  int tmp_ref_idx = ref_idx;
  uint tmp_ref_part = ref_part;
  bid_slt_pair* tmp_ref_part_ptr = ref_part_ptr;
  bid_slt_pair e;
  b_id my_bid = ref_part_ptr[ref_idx].bid;

  g_id gid;

  while (--lvl > 0)
  {
    e = tmp_ref_part_ptr[tmp_ref_idx];
    gid = get_first_gid(e.bid) | e.slt;

    if (e.bid == my_bid)
    {
      if ((ie_bs(e.slt, ie_list_begin, ie_list_end)) != NULL)
        return true;
    }
    else
    {
      if ((ee_bs(gid, ee_list_end, ee_list_end)) != NULL)
        return true;
    }

    if (++tmp_ref_idx == part_size)
    {
      if (++tmp_ref_part > mem_parts.size())
        tmp_ref_part_ptr = (bid_slt_pair*) mem_parts[tmp_ref_part];
      tmp_ref_idx = 0;
    }
  }

  return false;
}

template struct bufmgr3<bv_list_slt_start_with>; 
template struct bufmgr3<bv_list_slt_seq>; 
template struct bufmgr3<bv_list_slt>; 