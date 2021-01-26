#include "ml.h"

#include "util.h"
#include "util_t.h"

/* See dissertation Section 6.5 */
//#include <g-store.h>

void project(graph1* graph, int* v_per_p, int* v_per_p_begin, std::vector<bool>* part_type, int &max_p1)
{
  int num_v      = graph->num_v;
  int* offset_e  = graph->offset_e;
  int* v_w       = graph->v_w;
  int* e_to      = graph->e_to;
  int* e_w       = graph->e_w;
  int* map_in_c  = graph->map_in_c;
  int c_level    = graph->c_level;

  int c__num_v  = graph->coarser->num_v;
  int c__num_p  = graph->coarser->num_p;
  int* c__part  = graph->coarser->part;
  int* c__p_w   = graph->coarser->p_w;

  int i, j, k;
  int tmp, tmp2, w_l, w_r, cnt_v_l, cnt_v_r, w_p_l, w_p_r, cnt_p_l, cnt_v_p_l, cnt_v_p_r, cnt_p_r, cnt_isles = 0, cnt_offset = 0, avg_ps_w, num_ps;
  int cnt_parts = 0, cnt_p1s = 0;
  extemes extr;
  
  int8 threhold_w = get_threshold(c_level);
  
  int* part = graph->part = malloc1_set<int>(num_v, -1, "todo1", 2);
  int* v_tension = malloc1_set<int>(num_v, 0, "todo456", 2);       //TODO int8 for this :/
  
  print_debug_ln("projecting");

  for (i = 0; i < num_v; i++)
  {
    tmp = c__part[map_in_c[i]];
    
    for (j = offset_e[i]; j < offset_e[i+1]; j++)
    {
      tmp2 = c__part[map_in_c[e_to[j]]] - tmp;
    
      if (tmp2 != 0)
        v_tension[i] += e_w[j] * tmp2;  
    }
  }


  for (i = 0; i < c__num_p; i++)
  {
    if (GLOBALS::isle_map[cnt_isles] == i)
    {
      if (cnt_p1s > max_p1)
        max_p1 = cnt_p1s;

      cnt_p1s = 0; 

      GLOBALS::isle_map[cnt_isles++] = cnt_parts;
      
    }

    j = v_per_p_begin[i];
    
    // 1:1 map OR partition size < threshold
    
    if (v_per_p[j] == -1 || c__p_w[i] < threhold_w)
    {
      do 
        part[j] = cnt_parts;
      while ((j = v_per_p[j]) != -1);

      cnt_p1s++;
      
      print_debug("  %d->%d", i, cnt_parts);
      cnt_parts++;
      part_type->insert(part_type->end(), 1);
      continue;
    }


    // Partition size >= threshold
    num_ps = int(c__p_w[i] / threhold_w + (c__p_w[i] % threhold_w > 0 ? 0 : -1));
    avg_ps_w = int(c__p_w[i] / (c__p_w[i]/double(threhold_w)));
    
    w_l = w_r = cnt_v_l = cnt_v_r = 0;
    
    cnt_p_l = cnt_p_r = w_p_l = w_p_r = cnt_v_p_l = cnt_v_p_r = 0; 

    do 
    {
      part[j] = -2;                      
      extr.update(j, v_tension[j], -5);
    } while ((j = v_per_p[j]) != -1);
    
    for(;;)
    {
      if (extr.done())
        break;

      if (w_l < w_r || (w_l == w_r) && cnt_v_l <= cnt_v_r)
      {
        part[extr.min_idx] = cnt_parts + cnt_p_l;
        w_l += v_w[extr.min_idx];
        w_p_l += v_w[extr.min_idx];
        cnt_v_l++;
        cnt_v_p_l++;
        if (w_p_l > avg_ps_w)
        {
          w_p_l = cnt_v_p_l = 0;
          cnt_p_l++;
        }
        for (k = offset_e[extr.min_idx]; k < offset_e[extr.min_idx + 1]; k++)
          if (part[e_to[k]] <= -2)
          {
            v_tension[e_to[k]] -= e_w[k];
            if (part[e_to[k]] == -2 || part[e_to[k]] == -4)
              part[e_to[k]]--;
          }
      } 
      else
      {
        part[extr.max_idx] = cnt_parts + num_ps - cnt_p_r;
        w_r += v_w[extr.max_idx];
        w_p_r += v_w[extr.max_idx];
        cnt_v_r++;
        cnt_v_p_r++;
        if (w_p_r > avg_ps_w)
        {
          w_p_r = cnt_v_p_r = 0;
          cnt_p_r++;
        }
        for (k = offset_e[extr.max_idx]; k < offset_e[extr.max_idx + 1]; k++)
          if (part[e_to[k]] <= -2)
          {
            v_tension[e_to[k]] += e_w[k];
            if (part[e_to[k]] == -2 || part[e_to[k]] == -3)
              part[e_to[k]] -= 2;
          }
      }

      extr.reset();

      j = v_per_p_begin[i];
      do 
      extr.update(j, v_tension[j], part[j]);
      while ((j = v_per_p[j]) != -1);
    }
    
    if (cnt_v_p_l == 0)
      cnt_p_l--;

    if (cnt_v_p_r == 0)
      cnt_p_r--;

    if (num_ps - cnt_p_r != cnt_p_l && num_ps - cnt_p_r - 1 != cnt_p_l)
    {
      tmp = num_ps - cnt_p_r - cnt_p_l - 1;
      assert (tmp > 0);
            
      j = v_per_p_begin[i];
      do 
        if (part[j] > cnt_parts + cnt_p_l)
          part[j] -= tmp;
      while ((j = v_per_p[j]) != -1);

      num_ps -= tmp;
    }

    for (j = 0; j <= cnt_p_l; j++)
    {
      print_debug("  %d->%d", i, cnt_parts);
      cnt_parts++;
      part_type->insert(part_type->end(), 0);
    }

    for (; j <= num_ps; j++)
    {
      print_debug("  %d->%d", i, cnt_parts);
      cnt_parts++;
      part_type->insert(part_type->end(), 1);
    }
     
    cnt_p1s += cnt_p_l + 1;
    if (cnt_p1s > max_p1)
      max_p1 = cnt_p1s;

    cnt_p1s = cnt_p_r + 1;
  }

  if (cnt_p1s > max_p1)
    max_p1 = cnt_p1s;

  graph->num_p = cnt_parts;

  print_debug("\n");

  GLOBALS::isle_map [cnt_isles] = -1;
  free(v_tension);
}
