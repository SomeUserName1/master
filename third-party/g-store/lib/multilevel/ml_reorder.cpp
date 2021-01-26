#include "ml.h"

#include "util_t.h"
#include "util.h"

/* See dissertation Section 6.5 */
//#include <g-store.h>

// todo: implement an upper bound on upper_p1_bound
template <typename T_> T_* malloc1(int n, char *msg, int mode/* = 0*/);

void reorder(graph1* graph, int* v_per_p, int* v_per_p_begin, std::vector<bool>* part_type, int &max_p1)
{
  int num_v      = graph->num_v;
  int* offset_e  = graph->offset_e;
  int* v_w       = graph->v_w;
  int* e_to      = graph->e_to;
  int* e_w       = graph->e_w;
  int* map_in_c  = graph->map_in_c;
  int* part      = graph->part;
  int  num_p     = graph->num_p;
  
  if (max_p1 > 2047)
    max_p1 = 2047;

  int* part_mat_    = malloc1<int>((max_p1 + 2) * max_p1, "todo374", 1);
  int* old_new_map_ = malloc1<int>(max_p1 + 2, "todo374", 1);
  int* new_old_map_ = malloc1<int>(max_p1 + 2, "todo374", 1);
  int* tension_one  = malloc1<int>(max_p1, "todo374", 1);
  char* in_queue_   = malloc1<char>(max_p1 + 2, "todo495", 1);
  int** part_mat    = malloc1<int*>(max_p1, "todo5891", 1);
  void* pq_space    = malloc_b((sizeof(idx_sc_pair<int>) + sizeof(int)) * (max_p1 + 1), "todo324", 1);

  priority_queue1 pq(pq_space, max_p1);
  idx_sc_pair<int> top_elem;

  int i, j, k, m, n;
  int e_part, adj_left1, adj_right1, sw_left, sw_right, old_right, cnt_isles = 1;
  bool made_change;
  int* pm, *old_v_per_p_begin;

  *old_new_map_ = *new_old_map_ = -1;
  int* old_new_map = old_new_map_ + 1;
  int* new_old_map = new_old_map_ + 1;
  
  *in_queue_ = -1;
  char* in_queue = in_queue_ + 1;

  for (i = 0; i < max_p1; i++)
  {
    part_mat[i] = part_mat_ + (max_p1 + 2) * i + 1;
    *(part_mat[i] - 1) = 0;
  }

  //start
  
  for (i = 0; i < num_p - 1; i++)
  {
    if (i == GLOBALS::isle_map[cnt_isles])
      cnt_isles++;

    if (!(*part_type)[i])
      continue;

    /*if (!(i==0 || (*part_type)[i-1] == false || i == GLOBALS::isle_map[cnt_isles-1]))
      print_endln();*/

    k = 0;
    j = i + 1;
    while ((*part_type)[j])
      if (j == GLOBALS::isle_map[cnt_isles] || ++j == num_p || j-i >= 2047) //todo: change to max_p1
      {
        k = 1;
        break;
      }

    if (k == 0)
     while (!(*part_type)[j])
       if (j == GLOBALS::isle_map[cnt_isles] || ++j == num_p || j-i >= 2047) //todo: change to max_p1
         break;

    if (i == j - 1)
      continue;

    for (k = 0; k < j-i; k++)
    {
      //if (k >= max_p1)
        //print_endln();
      assert(k < max_p1);
      arr_set<int>(part_mat[k], 0, j - i + 1);
      old_new_map[k] = new_old_map[k] = k;
      tension_one[k] = 0;
    }

    made_change = false;

    old_new_map[j-i] = new_old_map [j-i] = -1;

    pq.reset();

    for (k = i; k < j; k++)
    {
      pm = part_mat[k-i];
      m = v_per_p_begin[k];

      do 
        for (n = offset_e[m]; n < offset_e[m+1]; n++)
        {
          if ((e_part = part[e_to[n]]) == k) 
            continue;

          if (e_part < k)
            tension_one[k-i] -= e_w[n];   
          else if (e_part > k)
            tension_one[k-i] += e_w[n];

          if (e_part >= i && e_part < j)
            pm[e_part-i] += e_w[n];
        }
      while ((m = v_per_p[m]) != -1);
      
    }

    for (k = 0; k < j-i-1; k++)
    {
      pq.insert(k, (tension_one[k] - part_mat[k][k+1]) - (tension_one[k+1] + part_mat[k+1][k]) ); //per move
      in_queue[k] = 1;
    }
    in_queue[j-i] = -1;

    for (;;)
    {
      top_elem = pq.pop_max();
      if (top_elem.sc > 0)
      {
        if (!made_change)
          made_change = true;

        sw_left    = old_new_map[top_elem.idx];
        sw_right   = sw_left + 1;
        old_right  = new_old_map[sw_right];
        adj_left1  = new_old_map[sw_left - 1];
        adj_right1 = new_old_map[sw_right + 1];

        tension_one[top_elem.idx]  -= 2 * part_mat[top_elem.idx][old_right];
        tension_one[old_right]     += 2 * part_mat[top_elem.idx][top_elem.idx];

        if (in_queue[adj_left1] == 1)
          pq.change(adj_left1,    (tension_one[adj_left1] - part_mat[adj_left1][old_right]) 
          - (tension_one[old_right] + part_mat[old_right][adj_left1]) );

        if (in_queue[adj_right1] == 1)
          pq.change(old_right,      (tension_one[top_elem.idx] - part_mat[top_elem.idx][adj_right1]) 
          - (tension_one[adj_right1]   + part_mat[adj_right1][top_elem.idx]) );

        swap1(old_new_map[top_elem.idx], old_new_map[old_right]);
        swap1(new_old_map[sw_left], new_old_map[sw_right]);
        in_queue[top_elem.idx] = 0;

      } 
      else 
        break;

      if (pq.empty())
        break;
    }


    if (made_change)
    {
      print_debug_ln("reordering:");
      for (k = 0; k < j-i; k++)
        print_debug("  %d->%d", k+i, i+old_new_map[k]);
      print_debug("\n");

      // reusing the unused memory
      old_v_per_p_begin = part_mat_;   
      
      for (k = i; k < j; k++)
        old_v_per_p_begin[k-i] = v_per_p_begin[k];

      for (k = i; k < j; k++)
        if (old_new_map[k-i] != k-i)
        {
          v_per_p_begin[k] = old_v_per_p_begin[old_new_map[k-i]];

          m = v_per_p_begin[k];

          do 
            part[m] = k;
          while((m = v_per_p[m]) != -1);
        }
    }

    i = j - 1;
  }

  free_all(7, part_mat_, old_new_map_, new_old_map_, tension_one, in_queue_, part_mat, pq_space);
}
