#include "ml.h"

#include "util_t.h"
#include "util.h"

/* See dissertation Section 6.3 */
//#include <g-store.h>

// return values:
//   0: coarsening ok
//   1: coarsening done (only islands)
int coarsen(graph1 *graph)
{
  static double c_ratio = 1;
  static int num_v_matched = 2;
  static int max_part_w = GLOBALS::blk_size;

  graph1* c_graph = malloc_graph1_init("todo912");
  
  graph->coarser = c_graph;
  c_graph->finer = graph;

start_over:
  if (num_v_matched == 2)
  {
    coarsen_two(graph, max_part_w, c_ratio);
    
    if (c_ratio == 0.0)
    { 
      free(c_graph);
      graph->coarser = NULL;
      return 1;
    }
    else if (c_ratio < 0.3)
      if (max_part_w <= 32 * GLOBALS::blk_size)
        max_part_w *= 2;
      else
        num_v_matched = 3;

    if (c_ratio == -1.0)
      goto start_over;

    assert(c_ratio > 0);
    GLOBALS::dbl_avg_c_ratio = ((GLOBALS::dbl_avg_c_ratio * graph->c_level) + c_ratio) / c_graph->c_level;
    return 0;
  }
  else
  {
    coarsen_n(graph, num_v_matched, c_ratio);

    if (c_ratio == 0.0)
    { 
      free(c_graph);
      graph->coarser = NULL;
      return 1;
    }

    if (c_ratio < 0.3)
      num_v_matched++;

    if (c_ratio == -1.0)
      goto start_over;

    assert(c_ratio > 0);
    GLOBALS::dbl_avg_c_ratio = ((GLOBALS::dbl_avg_c_ratio * graph->c_level) + c_ratio) / c_graph->c_level;
    return 0;
  }
}

// return values:
//  -1: ran out of memory
//   0: coarsening ok
void coarsen_two(graph1 *graph, int max_part_w, double &c_ratio)
{
  int num_v = graph->num_v;
  
  int* offset_e = graph->offset_e;
  int* v_w = graph->v_w;
  int* e_to = graph->e_to;
  int* e_w = graph->e_w;
  
  int* map_in_c = graph->map_in_c = malloc1_set<int>(num_v, -1, "coarsen: map_in_c", 2);
  
  int* v_per_p = malloc1<int>(num_v, "coarsen: match", 2);
  int* perm = malloc1<int>(num_v, "coarsen: perm", 2);

  int max_e_w, new_v = 0, max_v_w = 0, isles = 0;
  int i, j, v, v_matched;

  for (i = 0; i < num_v; i++)
    perm[i] = i;
  permute(perm, num_v);

  int* v_per_p_begin = perm;  // (2-in-1)

  /*
  for (i = 0; i < num_v; i++)
    if (v_w[i] > max_v_w) max_v_w = v_w[i];

  max_v_w = max_v_w/1000;

  //match small vertices with one edge first
  for (i = 0; i < num_v; i++) {
    v = perm[i];
    if (offset_e[v] + 1 == offset_e[v+1] && v_w[v] <= max_v_w)
    {
      if (match[v] == -1) {
        matched_v = v;

        j = offset_e[v];
        if (match[e_to[j]] == -1 && v_w[v] + v_w[e_to[j]] <= max_part_w) 
          matched_v = e_to[j];

        map_in_c[v] = map_in_c[matched_v] = new_v++;
        match[v] = matched_v;
        match[matched_v] = v;
      }
    }
  }
  */
  
  for (i = 0; i < num_v; i++) 
  {
    v = perm[i];

    if (map_in_c[v] == -1) 
    {
      v_matched = -1;
      max_e_w = 0;

      for (j = offset_e[v]; j < offset_e[v+1]; j++)
        if (map_in_c[e_to[j]] == -1 && max_e_w < e_w[j] && v_w[v] + v_w[e_to[j]] <= max_part_w)
        {
          v_matched = e_to[j];
          max_e_w = e_w[j];
        }

      v_per_p_begin[new_v] = v;
      if (v_matched == -1)
      {
        if (offset_e[v] == offset_e[v+1])
          isles++;
        v_per_p[v] = -1;
        map_in_c[v] = new_v++;
      }
      else
      { 
        v_per_p[v] = v_matched;
        v_per_p[v_matched] = -1;
        map_in_c[v] = map_in_c[v_matched] = new_v++;
      }
    }
  }
  
  //todo: if nothing is mathed bc of weight restriction, do not increase island
  if (num_v == new_v)
  {
    if (num_v == isles)
    {
      c_ratio = 0.0;
      graph->map_in_c = NULL;
    } 
    else
      c_ratio = -1.0;
    free_all(3, map_in_c, v_per_p, perm);
  }
  else
  {
    realloc1<int>(v_per_p_begin, new_v, "todo456", 2);
    
    create_c_graph(graph, new_v, v_per_p, v_per_p_begin);

    c_ratio = (num_v - new_v) / double(num_v - isles);

    free_all(2, v_per_p_begin, v_per_p);
  }
}

void coarsen_n(graph1* graph, int num_v_matched, double &c_ratio)
{
  int num_v = graph->num_v;
  int* offset_e = graph->offset_e;
  int* v_w = graph->v_w;
  int* e_to = graph->e_to;
  int* e_w = graph->e_w;

  int* map_in_c = graph->map_in_c = malloc1_set<int>(num_v, -1, "coarsen: map_in_c", 2);
  int* num_matched = malloc1_set<int>(num_v, 0, "coarsen: num_matched", 2);
  int* e_w_match = malloc1<int>(num_v, "coarsen: e_w_match", 2);
  int* perm = malloc1<int>(num_v, "coarsen: perm", 2);
  
  int* v_per_p, *v_per_p_begin, *v_per_p_end;

  int max_e_w, new_v = 0, max_v_w = 0, vp_matched;
  int i, j, tmp, v, isles = 0;
  bool to_v;

 for (i = 0; i < num_v; i++)
    perm[i] = i;
  permute(perm, num_v);

  /*
  for (i = 0; i < num_v; i++)
    if (v_w[i] > max_v_w) 
      max_v_w = v_w[i];

  max_v_w = max_v_w/1000;

  //match small vertices with one edge first
  for (i = 0; i < num_v; i++) {
    v = perm[i];
    if (offset_e[v] + 1 == offset_e[v+1] && v_w[v] <= max_v_w)
    {
      if (map_in_c[v] == -1) {
        v_matched = v;

        j = offset_e[v];
        if (match[e_to[j]] == -1 && v_w[v] + v_w[e_to[j]] <= max_part_w) 
          v_matched = e_to[j];

        map_in_c[v] = map_in_c[v_matched] = new_v++;
        match[v] = v_matched;
        match[v_matched] = v;
      }
    }
  }
  */

  for (i = 0; i < num_v; i++) 
  {
    v = perm[i];

    if (map_in_c[v] == -1) 
    {
      vp_matched = -1;
      max_e_w = 0;

      for (j = offset_e[v]; j < offset_e[v+1]; j++)
      {
        tmp = map_in_c[e_to[j]];
        if (tmp != -1 && num_matched[tmp] < num_v_matched)
          e_w_match[tmp] = 0;
      }

      for (j = offset_e[v]; j < offset_e[v+1]; j++)
      {
        tmp = map_in_c[e_to[j]];
        if (tmp == -1)
        {
          if (e_w[j] > max_e_w)
          {
            max_e_w = e_w[j];
            vp_matched = e_to[j];
            to_v = true;
          }
        }
        else if (num_matched[tmp] < num_v_matched)
        {
          e_w_match[tmp] += e_w[j];
          if (e_w_match[tmp] > max_e_w)
          {
            max_e_w = e_w_match[tmp];
            vp_matched = tmp;
            to_v = false;
          }
        }
      }
      
      if (vp_matched == -1)
      {
        if (offset_e[v] == offset_e[v+1])
          isles++;
        map_in_c[v] = new_v++;
      }
      else if (to_v)
      {
        map_in_c[v] = map_in_c[vp_matched] = new_v;
        num_matched[new_v++] = 2;
      }
      else
      {
        map_in_c[v] = vp_matched;
        num_matched[vp_matched]++;
      }
    }
  }

  if (num_v == new_v)
  {
    if (num_v == isles)
    {
      c_ratio = 0.0;
      graph->map_in_c = NULL;
    }
    else
      c_ratio = -1.0;
    free_all(4, map_in_c, num_matched, e_w_match, perm);       
  }
  else
  {
    arr_set<int>(v_per_p = perm, -1, num_v);
    arr_set<int>(v_per_p_end = num_matched, -1, new_v);   // (need only new_v)

    free(e_w_match);
    v_per_p_begin = malloc1<int>(new_v,"todo879", 2);

    for (i = 0; i < num_v; i++)
    {
      tmp = map_in_c[i];
      if (v_per_p_end[tmp] == -1)
        v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
      else
      {
        v_per_p[v_per_p_end[tmp]] = i;
        v_per_p_end[tmp] = i;
      }
    }

    free(v_per_p_end);

    create_c_graph(graph, new_v, v_per_p, v_per_p_begin);

    c_ratio = (num_v - new_v) / double(num_v - isles);  

    free_all(2, v_per_p_begin, v_per_p);
  }
};

void create_c_graph(graph1* graph, int new_v, int* v_per_p, int* v_per_p_begin)
{
  graph1* c_graph = graph->coarser;
  c_graph->num_v = new_v;

  int num_v     = graph->num_v;
  int num_e     = graph->num_e;
  int* offset_e = graph->offset_e;
  int* v_w      = graph->v_w;
  int* e_to     = graph->e_to;
  int* e_w      = graph->e_w;
  int* map_in_c = graph->map_in_c;

  int* c__offset_e  = c_graph->offset_e =  malloc1<int>(new_v + 1, "cgraph->offset_e in create_initial_cgraph()", 2);
  int* c__v_w       = c_graph->v_w = malloc1<int>(new_v, "cgraph->v_w in create_initial_cgraph()", 2);  
  int* c__e_to;
  int* c__e_w;

  c_graph->num_v    = new_v;
  c_graph->c_level  = graph->c_level + 1;

  int* htable = malloc1_set<int>(new_v, -1, "todo465", 2);
  
  int i, j, k, tmp, cnt_e = 0;
  
  // Just counts number of edges
  //  One alternative would be to allocate space for 2 * num_e and realloc after done
  //  Another alternative would be to allocate space for c_ratio * 2 * num_e and realloc as necessary
  for (i = 0; i < new_v; i++)
  {    
    htable[i] = i;
    
    j = v_per_p_begin[i];
    for (;;)
    {
      for (k = offset_e[j]; k < offset_e[j+1]; k++)
      {
        tmp = map_in_c[e_to[k]];
        if (htable[tmp] != i)
        {
          htable[tmp] = i;
          cnt_e++;
        }
      }

      if ((j = v_per_p[j]) == -1)
        break;
    }
  }

  c_graph->num_e = cnt_e;
  
  c__e_to = c_graph->e_to = malloc1<int>(cnt_e, "todo4566", 2);  
  c__e_w = c_graph->e_w = malloc1<int>(cnt_e, "todo897", 2);
  
  arr_set<int>(htable, -1, new_v);
  cnt_e = 0;

  // Actually create c_graph
  for (i = 0; i < new_v; i++)
  {  
    c__offset_e[i] = cnt_e;
    
    j = v_per_p_begin[i];
    
    c__v_w[i] = v_w[j];

    for(;;)
    {
      for (k = offset_e[j]; k < offset_e[j+1]; k++)
      {
        if ((tmp = map_in_c[e_to[k]]) == i)
          continue;

        if (htable[tmp] == -1)
        {
          htable[tmp] = cnt_e;
          c__e_to[cnt_e] = tmp;
          c__e_w[cnt_e++] = e_w[k];
        }
        else
          c__e_w[htable[tmp]] += e_w[k];
      }
      
      if ((j = v_per_p[j]) == -1)
        break;
      
      c__v_w[i] += v_w[j];
    }

    for (j = c__offset_e[i]; j < cnt_e; j++)
      htable[c__e_to[j]] = -1;
  }
  
  c__offset_e[new_v] = cnt_e;
  
  free(htable);
}