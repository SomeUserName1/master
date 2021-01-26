#include "ml.h"

#include "util_t.h"
#include "util.h"
#include "parameters.h"

/* See dissertation Section 6.5 */
//#include <g-store.h>

void refine(graph1* graph, int* v_per_p, int* v_per_p_begin)
{
  static int max_p = 256;
  static int max_v = 256;
  static int pq_size = max_p * max_v / 10;

  int num_v      = graph->num_v;
  int* offset_e  = graph->offset_e;
  int* v_w       = graph->v_w;
  int* e_to      = graph->e_to;
  int* e_w       = graph->e_w;
  int* map_in_c  = graph->map_in_c;
  int* part      = graph->part;
  int  num_p     = graph->num_p;
  int  c_level   = graph->c_level;
  
  int* p_w = graph->p_w;

  int* idx_v             = malloc1<int>(max_v, "10001", 1);
  int* idx_p             = malloc1<int>(max_p, "10002", 1);
  int* cnt_ps            = malloc1<int>(max_p, "10003", 1);					
  void* pq_space         = malloc_b((max_v + 1) * sizeof(int) + max_v * sizeof(idx_p_sc), "10004", 1);
  int* perm              = malloc1<int>(num_p, "10004", 1);                   
  int* part_map          = malloc1_set<int>(num_p, -1, "10005", 1); 

  int* a_cnt_l              = malloc1<int>(max_v, "10006", 1); 
  int* a_cnt_r              = malloc1<int>(max_v, "10007", 1); 
  int** a_plist             = malloc1<int*>(max_v + 1, "10008", 1); 
  int* a_plist_             = malloc1<int>((max_v + 1)* (max_p + 1), "10009", 1); 
  idx_scd_w_pair** a_poswsc = malloc1<idx_scd_w_pair*>(max_v, "10010", 1); 
  idx_scd_w_pair* a_poswsc_ = malloc1<idx_scd_w_pair>(max_v * max_p, "10011", 1); 
  int* a_iw                 = malloc1<int>(max_v, "10012", 1); 

  std::vector<std::vector<bool> > p_mat;
  p_mat.resize(num_p, std::vector<bool>(num_p));
  std::vector<bool>* p_mat_p;

  priority_queue2 pq;
  pq.init(pq_space, max_v);
  
  int8 threshold_w = get_threshold(c_level), alpha_sc;
  int give_up_threshold = (int) (PARAM::give_up_page_multiple * GLOBALS::blk_writable);
  int i, j, k, m, n, p, r, runs, best_p, best_v, best_v_idx, best_p_idx, beta_sc, gamma_sc;
  int cnt_v, cnt_p, cnt_isles, tmp, tmp_sc_p, tmp_p, tmp_p1, tmp_v;
  double best_sc, tmp_sc;
  int* found;
  idx_scd_w_pair* a_poswsc_v;
  int* a_plist_v;

  int sc_sm_m = 1;  //for c_levels >= 8
  int sc_lg_m = 8;
  int8 max_p_w;

  if (c_level < 8)
  {
    runs = PARAM::runs_b;
    sc_sm_m += 2 * (8 - c_level);
    sc_lg_m += 64 * (int) pow(2.0, 8 - c_level);
    max_p_w = threshold_w * 8; //* c_level + 1;
  }
  else
  {
    runs = PARAM::runs_a;
    max_p_w = threshold_w * 8;
  }
  
  //runs = 1;   //TBD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  max_p_w = threshold_w;

  for (i = 0; i < num_v; i++)
    p_w[part[i]] += v_w[i];      //stored in graph

  //prep-work
  for (i = 0; i < max_v; i++)
  {  
    a_plist[i] = a_plist_ + (max_p + 1) * i;
    a_poswsc_v = a_poswsc[i] = a_poswsc_ + max_p * i;
    for (j = 0; j < max_p; j++)
      a_poswsc_v[j].idx = -1;
    a_iw[i] = 0;
    a_cnt_l[i] = 0;
    a_cnt_r[i] = max_p;
  }

  for (i = 0; i < num_p; i++)
  {
    p_mat_p = &p_mat[i];
    for (j = 0; j < num_p; j++)
      (*p_mat_p)[j] = false;
  }

  for (i = 0; i < num_p; i++)
  {
    if ((j = v_per_p_begin[i]) == -1)
      continue;

    p_mat_p = &p_mat[i];

    do 
    {
      //assert(part[j] == i);       //todo: delete!
      for (k = offset_e[j]; k < offset_e[j+1]; k++)
        (*p_mat_p)[part[e_to[k]]] = true;
    } while ((j = v_per_p[j]) != -1);
  } 

  //start
  for (r = 0; r < runs; r++)
  {
    for (i = 0; i < num_p; i++)
      perm[i] = i;
    permute(perm, num_p);
    
    for (i = 0; i < num_p; i++)
    {
      p = perm[i];
      
      if ((j = v_per_p_begin[p]) == -1) //empty
        continue;

      cnt_v = cnt_p = 0;

      do 
      {
        idx_v[cnt_v++] = j;

        if (cnt_v == max_v)
        {
          max_v += 256;
          idx_v      = realloc1<int>(idx_v, max_v, "10013", 1);
          free_all(8, pq_space, a_cnt_l, a_cnt_r, a_plist, a_plist_, a_poswsc, a_poswsc_, a_iw);
          pq_space   = malloc_b((max_v + 1) * sizeof(int) + max_v * sizeof(idx_p_sc), "10014", 1);
          a_cnt_l    = malloc1<int>(max_v, "10015", 1); 
          a_cnt_r    = malloc1<int>(max_v, "10016", 1); 
          a_plist    = malloc1<int*>(max_v + 1, "10017", 1); 
          a_plist_   = malloc1<int>((max_v + 1)* (max_p + 1), "10018", 1); 
          a_poswsc   = malloc1<idx_scd_w_pair*>(max_v, "10019", 1); 
          a_poswsc_  = malloc1<idx_scd_w_pair>(max_v * max_p, "10020", 1); 
          a_iw       = malloc1<int>(max_v, "10021", 1); 
          pq.init(pq_space, max_v);
          for (k = 0; k < max_v; k++)
          {
            a_plist[k] = a_plist_ + (max_p + 1) * k;
            a_poswsc_v = a_poswsc[k] = a_poswsc_ + max_p * k;
            for (m = 0; m < max_p; m++)
              a_poswsc_v[m].idx = -1;
            a_iw[k] = 0;
            a_cnt_l[k] = 0;
            a_cnt_r[k] = max_p;
          }
        }

        for (k = offset_e[j]; k < offset_e[j+1]; k++)
        {
          tmp = part[e_to[k]];
          if (tmp != p && part_map[tmp] == -1)
          {
            idx_p[cnt_p] = tmp;
            part_map[tmp] = cnt_p++;
            if (cnt_p == max_p)
            {
              max_p += 256;
              idx_p = realloc1<int>(idx_p, max_p, "10022", 1);
              free_all(4, cnt_ps, part_map, a_plist_, a_poswsc_);
              cnt_ps      = malloc1<int>(max_p, "10023", 1);
              part_map    = malloc1_set<int>(num_p, -1, "10024", 1); 
			  a_plist_    = malloc1<int>((max_v + 1)* (max_p+1), "10025", 1); 
              a_poswsc_   = malloc1<idx_scd_w_pair>(max_v * max_p, "10026", 1);
              for (m = 0; m < max_v; m++)
              {
                a_plist[m] = a_plist_ + (max_p + 1) * m;
                a_poswsc[m] = a_poswsc_ + max_p * m;
                a_poswsc_v = a_poswsc[m];
                for (n = 0; n < max_p; n++)
                  a_poswsc_v[n].idx = -1;
                a_cnt_r[m] = max_p;
              }
            }
          }
        }
      } while((j = v_per_p[j]) != -1);
        
      if (cnt_p == 0)
        continue;
      
      qsort(idx_p, cnt_p, sizeof(int), bs_comp_int<int>);    // (idx_v already sorted)
      
      for (j = 0; j < cnt_p; j++)
      {
        part_map[idx_p[j]] = j;      
        cnt_ps[j] = 0;
      }
  
      for (j = 0; j < cnt_v; j++)
      {
        for (k = offset_e[idx_v[j]]; k < offset_e[idx_v[j]+1]; k++)
          if (part[e_to[k]] != p && cnt_ps[part_map[part[e_to[k]]]] >= 0)
            cnt_ps[part_map[part[e_to[k]]]] = -cnt_ps[part_map[part[e_to[k]]]] - 1;
        
       for (k = 0; k < cnt_p; k++)
         if (cnt_ps[k] < 0)
           cnt_ps[k] = -cnt_ps[k];
      }
     
      for (j = 0; j < cnt_v; j++)
      {
        //create a_plist, a_poswsc
        a_poswsc_v = a_poswsc[j];
        a_plist_v = a_plist[j];
        for (k = offset_e[idx_v[j]]; k < offset_e[idx_v[j]+1]; k++) 
        {
          tmp_p = part[e_to[k]];
          if (tmp_p == p)
            a_iw[j] += e_w[k];
          else
          {
            tmp = part_map[tmp_p];
            if (a_poswsc_v[tmp].idx == -1)
            {
              a_poswsc_v[tmp].w = e_w[k];
              a_poswsc_v[tmp].idx = a_cnt_l[j];
			  a_plist_v[a_cnt_l[j]] = tmp;
              a_cnt_l[j]++;
            }
            else
              a_poswsc_v[tmp].w += e_w[k];
          }
        }

        k = 0;
        tmp_sc_p = -1;
        tmp_sc = MININT;
        while(k < a_cnt_l[j])
        {
          tmp_p = a_plist_v[k];
          if (p_w[idx_p[tmp_p]] + v_w[idx_v[j]] > max_p_w)
          {
            a_plist_v[a_cnt_r[j]] = tmp_p;
            a_poswsc_v[tmp_p].idx = a_cnt_r[j];
            a_poswsc_v[tmp_p].sc = -1.0;
            a_cnt_r[j]--;
            a_cnt_l[j]--;
            if (k == a_cnt_l[j])
              break; //else:
            
            tmp_p1 = a_plist_v[a_cnt_l[j]];
            a_plist_v[k] = tmp_p1;
            a_poswsc_v[tmp_p1].idx = k;
          }
          else
          {
            alpha_sc = std::abs(p - idx_p[tmp_p]) * -a_iw[j];  //p is not in plist, a_poswsc_v[tmp_p].w considered below
            beta_sc = a_poswsc_v[tmp_p].w - a_iw[j];
            for (m = 0, gamma_sc = 0; m <= max_p; m++)
            {
              if (m == a_cnt_l[j]) { m = a_cnt_r[j] + 1; if (m > max_p) break; }

              tmp_p1 = a_plist_v[m];
              alpha_sc += a_poswsc_v[tmp_p1].w * (std::abs(p - idx_p[tmp_p1]) - std::abs(idx_p[tmp_p] - idx_p[tmp_p1]));

              if (cnt_ps[tmp_p1] == 1)
                gamma_sc += 1;
              if (tmp_p1 != tmp_p && !p_mat[idx_p[tmp_p]][idx_p[tmp_p1]])
                gamma_sc -= 1;
            }
            
            if (r == 0 && (alpha_sc < 0 || beta_sc < 0 || gamma_sc < 0))
            {
              a_plist_v[a_cnt_r[j]] = tmp_p;
              a_poswsc_v[tmp_p].idx = a_cnt_r[j];
              a_poswsc_v[tmp_p].sc = -1.0;
              a_cnt_r[j]--;
              a_cnt_l[j]--;
              if (k == a_cnt_l[j])
                break; //else:
              
              tmp_p1 = a_plist_v[a_cnt_l[j]];
              a_plist_v[k] = tmp_p1;
              a_poswsc_v[tmp_p1].idx = k;
            }
            else
            {
              a_poswsc_v[tmp_p].sc = PARAM::alpha * alpha_sc + PARAM::beta * beta_sc + PARAM::gamma * gamma_sc + 
                (p_w[idx_p[tmp_p]] < GLOBALS::blk_writable ? sc_sm_m : - sc_lg_m * (int) (p_w[idx_p[tmp_p]] / threshold_w)); 
                  //consider: p_w + v_w

              if (a_poswsc_v[tmp_p].sc > tmp_sc)
              {
                tmp_sc = a_poswsc_v[tmp_p].sc;
                tmp_sc_p = tmp_p;
              }
              k++;
            }
          }
        }
        if (tmp_sc_p != -1)
          pq.insert(j, tmp_sc_p, tmp_sc);
        else
          pq.insert_r(j);
      } //next vertex (next j)       
      
      while (!pq.empty())
      {
        pq.pop_max(best_v_idx, best_p_idx, best_sc);
        
        assert(part[idx_v[best_v_idx]] == p);

        best_p = idx_p[best_p_idx];
        best_v = idx_v[best_v_idx];
        a_poswsc_v = a_poswsc[best_v_idx];
        a_plist_v = a_plist[best_v_idx];

        if (best_sc <= (double) (p_w[p] <= give_up_threshold ? - sc_sm_m : (p_w[p] <= GLOBALS::blk_size ? sc_sm_m : 0)))
        {
          //clear
          for (j = 0; j <= max_p; j++)
          {
            if (j == a_cnt_l[best_v_idx]) { j = a_cnt_r[best_v_idx] + 1; if (j > max_p) break; }
            a_poswsc_v[a_plist_v[j]].idx = -1;
          }
          a_cnt_l[best_v_idx] = 0;
          a_cnt_r[best_v_idx] = max_p;
          a_iw[best_v_idx] = 0;
          break;
        }
                  
        if (p_w[best_p] + v_w[best_v] > max_p_w)
        { 
          tmp = a_poswsc_v[best_p_idx].idx;              //tmp = plist position before move
          a_plist_v[a_cnt_r[best_v_idx]] = best_p_idx;
          a_poswsc_v[best_p_idx].idx = a_cnt_r[best_v_idx];
          a_poswsc_v[best_p_idx].sc = -1.0;
          a_cnt_r[best_v_idx]--;
          a_cnt_l[best_v_idx]--;
          if (tmp != a_cnt_l[best_v_idx])
          {
            a_plist_v[tmp] = a_plist_v[a_cnt_l[best_v_idx]];
            a_poswsc_v[a_plist_v[tmp]].idx = tmp;
          }

          tmp_sc = MININT;
          tmp_sc_p = -1;
          for (j = 0; j < a_cnt_l[best_v_idx]; j++)
            if (a_poswsc_v[a_plist_v[j]].sc > tmp_sc)
              {
                tmp_sc = a_poswsc_v[a_plist_v[j]].sc;
                tmp_sc_p = a_plist_v[j];
              }
          if (tmp_sc_p != -1)
            pq.insert(best_v_idx, tmp_sc_p, tmp_sc);
          else
            pq.insert_r(best_v_idx);
          
          continue;
        } //else:
        
        //print_ln("moving (%d) %d: %d -> %d (%4f; %d-%d->%d)", 
        //  r, best_v, p, best_p, best_sc, p_w[p], v_w[best_v], p_w[best_p]);

        part[best_v] = best_p;
        p_w[p] -= v_w[best_v];
        p_w[best_p] += v_w[best_v];
        
        change_p_lists(best_v, p, best_p, v_per_p, v_per_p_begin); 

        for (j = 0; j <= max_p; j++)
        {
          if (j == a_cnt_l[best_v_idx]) { j = a_cnt_r[best_v_idx] + 1; if (j > max_p) break; }
          
          
          tmp_p = a_plist_v[j];
          if (tmp_p == best_p_idx)
            continue;             //treated after internal

          // Gamma, Case A - chance to clear link
          if (--cnt_ps[tmp_p] == 1)
          {
            for (k = 0; k < cnt_v; k++)
              if (a_poswsc[k][tmp_p].idx != -1 && a_poswsc[k][tmp_p].idx < a_cnt_l[k] && k != best_v_idx)
              {
                a_poswsc[k][tmp_p].sc += PARAM::gamma;
                pq.change_if_better(k, tmp_p, a_poswsc[k][tmp_p].sc);
                break;
              }
          }
          else if (cnt_ps[tmp_p] == 0)
            p_mat[p][idx_p[tmp_p]] = false;

          // Gamma, Case B - update links from best_p, maybe take back penalty of move to best_p
          if (!p_mat[best_p][idx_p[tmp_p]])
          {
            p_mat[best_p][idx_p[tmp_p]] = true;
            for (k = 0; k < cnt_v; k++)
              if (a_poswsc[k][best_p_idx].idx != -1 && a_poswsc[k][tmp_p].idx != -1 && a_poswsc[k][best_p_idx].idx < a_cnt_l[k] && k != best_v_idx)
              {
                a_poswsc[k][best_p_idx].sc += PARAM::gamma;
                pq.change_if_better(k, best_p_idx, a_poswsc[k][best_p_idx].sc);
              }
          }

          // Score
          tmp = 0;
          if (p_w[best_p] - v_w[best_v] < GLOBALS::blk_writable && p_w[best_p] > GLOBALS::blk_writable)
            tmp = sc_sm_m;
          else if ((int) ((p_w[best_p] - v_w[best_v]) / threshold_w) < (int)(p_w[best_p] / threshold_w))
            tmp = sc_lg_m;

          if (tmp != 0)
          {
            for (k = 0; k < cnt_v; k++)
              if (a_poswsc[k][best_p_idx].idx != -1 && a_poswsc[k][best_p_idx].idx < a_cnt_l[k] && k != best_v_idx)
              {
                a_poswsc[k][best_p_idx].sc -= tmp;
                if (pq.pos[k].p == best_p_idx)
                {
                  a_poswsc_v = a_poswsc[k];
                  a_plist_v = a_plist[k];
                  tmp_sc = MININT;
                  tmp_sc_p = -1;
                  for (m = 0; m < a_cnt_l[k]; m++)
                    if (a_poswsc_v[a_plist_v[m]].sc > tmp_sc)
                    {
                      tmp_sc = a_poswsc_v[a_plist_v[m]].sc;
                      tmp_sc_p = a_plist_v[m];
                    }
                    assert (tmp_sc_p != -1);
                  pq.change(k, tmp_sc_p, tmp_sc);
                }
              }
          }
        }
        
        cnt_ps[best_p_idx]--;

        // Alpha, Beta
        for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
        {
          if (part[e_to[j]] == p)
          {
            cnt_ps[best_p_idx]++;

            found = (int*) bsearch(&e_to[j], idx_v, cnt_v, sizeof(int), bs_comp_int<int>);
            assert(found != NULL);
            tmp_v = found - idx_v;
            a_poswsc_v = a_poswsc[tmp_v];
            a_plist_v = a_plist[tmp_v];

            if (a_poswsc_v[best_p_idx].idx == -1) 
            {
              a_poswsc_v[best_p_idx].w = 0;   //as if best_v was not transfered yet

              if (p_w[best_p] + v_w[tmp_v] > max_p_w)
              {
                a_plist_v[a_cnt_r[tmp_v]] = best_p_idx;
                a_poswsc_v[best_p_idx].idx = a_cnt_r[tmp_v];
                a_poswsc_v[best_p_idx].sc = -1.0;
                a_cnt_r[tmp_v]--;
              }
              else
              {
                alpha_sc = -std::abs(p - best_p) * a_iw[tmp_v];  //corrected later
                beta_sc = -a_iw[tmp_v];

                for (k = 0, gamma_sc = 0; k <= max_p; k++)
                {
                  if (k == a_cnt_l[tmp_v]) { k = a_cnt_r[tmp_v] + 1; if (k > max_p) break; }

                  tmp_p = a_plist_v[k];
                  alpha_sc += a_poswsc_v[tmp_p].w * (std::abs(p - idx_p[tmp_p]) - std::abs(best_p - idx_p[tmp_p]));

                  if (best_p_idx != tmp_p)
                  {
                    if (cnt_ps[tmp_p] == 1)   //best_p == idx_p[tmp_p] treated below
                      gamma_sc += 1;
                    if (!p_mat[best_p][idx_p[tmp_p]]) 
                      gamma_sc -= 1;
                  }
                }
                
                if (r == 0 && (alpha_sc < 0 || beta_sc < 0 || gamma_sc < 0))
                {
                  a_plist_v[a_cnt_r[tmp_v]] = best_p_idx;
                  a_poswsc_v[best_p_idx].idx = a_cnt_r[tmp_v];
                  a_poswsc_v[best_p_idx].sc = -1.0;
                  a_cnt_r[tmp_v]--;
                }
                else
                {
                  if (a_cnt_l[tmp_v] == 0)
                  {
                    assert(pq.cnt_elem_r < max_v+1);
                    pq.remove_r(tmp_v);
                  }

                  a_plist_v[a_cnt_l[tmp_v]] = best_p_idx;
                  a_poswsc_v[best_p_idx].idx = a_cnt_l[tmp_v];
                  a_poswsc_v[best_p_idx].sc = PARAM::alpha * alpha_sc + PARAM::beta * beta_sc + PARAM::gamma * gamma_sc
                    + (p_w[best_p] < GLOBALS::blk_writable ? sc_sm_m : - sc_lg_m * (int) (p_w[best_p] / threshold_w));
                  a_cnt_l[tmp_v]++;

                  if (a_cnt_l[tmp_v] == 1)
                    pq.insert(tmp_v, best_p_idx, a_poswsc_v[best_p_idx].sc);

                }
              }
            } //if new end
            
            a_iw[tmp_v] -= e_w[j];
            a_poswsc_v[best_p_idx].w += e_w[j];
            
            tmp_sc_p = -1;
            tmp_sc = MININT; 

            for (k = 0; k < a_cnt_l[tmp_v]; k++)      //adjust score of moving tmp_v to k
            {
              tmp_p = a_plist_v[k];
              a_poswsc_v[tmp_p].sc += PARAM::alpha * std::abs(p - idx_p[tmp_p]) * e_w[tmp_v];
              a_poswsc_v[tmp_p].sc += PARAM::beta * e_w[tmp_v];
              // status now: as if edge had not existed

              if (tmp_p == best_p_idx)
                a_poswsc_v[tmp_p].sc +=  PARAM::beta * e_w[tmp_v];

              a_poswsc_v[tmp_p].sc += PARAM::alpha * e_w[tmp_v] * (std::abs(p - best_p) - std::abs(idx_p[tmp_p] - best_p));
                
              if (a_poswsc_v[tmp_p].sc > tmp_sc)
              {
                tmp_sc = a_poswsc_v[tmp_p].sc;
                tmp_sc_p = tmp_p;
              }
            }
            if (tmp_sc_p != -1)
              pq.change(tmp_v, tmp_sc_p, tmp_sc);
          }
        }
        
        // Gamma, Case C - only one link left to best_p_idx
        if (cnt_ps[best_p_idx] == 1)
        {
          for (k = 0; k < cnt_v; k++)
            if (a_poswsc[k][best_p_idx].idx != -1 && k != best_v_idx)
            {
              if (a_poswsc[k][best_p_idx].idx < a_cnt_l[k])
              {
                a_poswsc[k][best_p_idx].sc += PARAM::gamma;
                pq.change_if_better(k, best_p_idx, a_poswsc[k][best_p_idx].sc);
              }
              break;
            }
        }
        else if (cnt_ps[best_p_idx] == 0)
          p_mat[p][best_p] = false;

        //clear
        a_poswsc_v = a_poswsc[best_v_idx];
        a_plist_v = a_plist[best_v_idx];
        for (j = 0; j <= max_p; j++)
        {
          if (j == a_cnt_l[best_v_idx]) { j = a_cnt_r[best_v_idx] + 1; if (j > max_p) break; }
          a_poswsc_v[a_plist_v[j]].idx = -1;
        }
        a_cnt_l[best_v_idx] = 0;
        a_cnt_r[best_v_idx] = max_p;
        a_iw[best_v_idx] = 0;

      } // end while !pq.empty 


      for (j = 1; j <= max_v; j++)
      {
        if (j == pq.cnt_elem + 1) { j = pq.cnt_elem_r; if (j > max_v) break; }
        tmp_v = pq.pq[j];
        a_poswsc_v = a_poswsc[tmp_v];
        a_plist_v = a_plist[tmp_v];
        for (k = 0; k <= max_p; k++)
        {
          if (k == a_cnt_l[tmp_v]) { k = a_cnt_r[tmp_v] + 1; if (k > max_p) break; }
          a_poswsc_v[a_plist_v[k]].idx = -1;
        }
        a_cnt_l[tmp_v] = 0;
        a_cnt_r[tmp_v] = max_p;
        a_iw[tmp_v] = 0;
      }
      pq.reset();

      /*for (j = 0; j < max_v; j++)
        for (k = 0; k < max_p; k++)
          if (a_poswsc[j][k].idx != -1)
            print_endln();*/

      for (j = 0; j < cnt_p; j++)
        part_map[idx_p[j]] = -1;

	  } // partition done!

    // might have to be moved inside loop to get a correct estimate of distance
    tmp = cnt_isles = n = 0;
    
    for (i = 0; i < num_p; i++)
    {
      if (v_per_p_begin[i] == -1)
      {
        assert(p_w[i] == 0);
        if (GLOBALS::isle_map[cnt_isles] == i)
        {
          assert(n==0);
          n = 1;
        }
        tmp++;
        continue;
      }

      if (i - tmp > 0 && p_w[i-tmp-1] + p_w[i] <= GLOBALS::blk_size && GLOBALS::isle_map[cnt_isles] != i && n == 0)
      {
        tmp++;
        j = v_per_p_begin[i-tmp];
        k = v_per_p_begin[i];

        if (k < j)
        {
          v_per_p_begin[i-tmp] = m = k;
          part[k] -= tmp;
          k = v_per_p[k];
        } 
        else
        {
          m = j;
          j = v_per_p[j];
        }

        for(;;)
        {
          if (j == -1)
          {
            v_per_p[m] = k;
            do 
            part[k] -= tmp;
            while ((k = v_per_p[k]) != -1);
            break;
          }

          if (k == -1)
          {
            v_per_p[m] = j;
            break;
          }

          if (j < k)
          {
            v_per_p[m] = j;
            m = j;
            j = v_per_p[j];
          }
          else
          {
            v_per_p[m] = k;
            m = k;
            part[k] -= tmp;
            k = v_per_p[k];
          }
        }

        p_w[i-tmp] += p_w[i];
        continue;
      }

      if (tmp > 0)
      {
        if(GLOBALS::isle_map[cnt_isles] == i || n == 1)
        {
          GLOBALS::isle_map[cnt_isles++] = i - tmp;
          n = 0;
        }

        j = v_per_p_begin[i-tmp] = v_per_p_begin[i];
      
        do 
          part[j] -= tmp;
        while ((j = v_per_p[j]) != -1);
      
        p_w[i-tmp] = p_w[i];
      }

      if (GLOBALS::isle_map[cnt_isles] == i)
        cnt_isles++;
    }

    num_p -= tmp;
    graph->num_p = num_p;
    GLOBALS::isle_map [cnt_isles] = -1;
  }

  free_all(13, idx_v, idx_p, cnt_ps, pq_space, perm, part_map, a_cnt_l, a_cnt_r, a_plist, a_plist_, a_poswsc, a_poswsc_, a_iw);
}
