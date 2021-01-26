#include "ml.h"

#include "util_t.h"
#include "util.h"
#include "block.h"

/* See dissertation Section 6.6  */
//#include <g-store.h>

void finalize(graph1* graph, int* &v_per_p, int* &v_per_p_begin)
{

  int num_v = graph->num_v;
  int num_e = graph->num_e;
  int num_p = graph->num_p;
  int* e_w = graph->e_w;       
  int* v_w = graph->v_w;       
  int* offset_e = graph->offset_e;       
  int* e_to = graph->e_to;      
  int* part = graph->part;      
  int* p_w = graph->p_w;       
  int* org_size = malloc1<int>(num_v,"todo");
  int* p_num_v = malloc1<int>(num_p,"todo");
  int* v_map = malloc1<int>(num_v, "todo");

  int i, j, k, m, n, tmp, tmp2, max_num_rec = 0;

  int my_p_w, l_p_w, r_p_w;
  int my_num_v, l_num_v, r_num_v;
  bool l_go, r_go, success;
  int new_my_p_w, new_lr_p_w, cnt_new_p, best_v;
  int cnt_v, new_p, p_add = 0;
  int ee_ie_diff;

  idx_sc_pair<int> max_to_l;
  idx_sc_pair<int> max_to_r;

  fread(org_size, sizeof(int), num_v, GLOBALS::fp_org_size);
  fclose(GLOBALS::fp_org_size);

  tmp = 0;
  for (i = 0; i < num_p; i++)
  {
    cnt_v = 0;

    j = v_per_p_begin[i];
    do 
      cnt_v++;
    while ((j = v_per_p[j]) != -1);

    p_num_v[i] = cnt_v;
    if (cnt_v >= 256)
      tmp++;
    
    if (cnt_v > max_num_rec)
      max_num_rec = cnt_v;
  }

  // the parameters will be set based on the first run of vertices
  if (FIRST_RUN)
  {
    if (tmp > 0.005 * num_p)
    {
      GLOBALS::ie_size = 2;
      GLOBALS::blk_max_num_rec = UINT2_MAX + 1;
    }
    else
    {
      GLOBALS::ie_size = 1;
      GLOBALS::blk_max_num_rec = UINT1_MAX + 1;
    }

    GLOBALS::max_rec_p_page = GLOBALS::blk_max_num_rec;

    ee_ie_diff = EE_SIZE - GLOBALS::ie_size;
  
    GLOBALS::blk_writable = GLOBALS::blk_size - (4 + GLOBALS::header_slt_len + GLOBALS::ie_size * 2);
  }

  for (i = 0; i < num_p; i++)
  {
    tmp = 0;
    j = v_per_p_begin[i];

    do 
    {
      tmp2 = 0;
      for (k = offset_e[j]; k < offset_e[j] + org_size[j]; k++)
        if (part[e_to[k]] == i)
          tmp2 += ee_ie_diff;

      tmp += tmp2;
    } while ((j = v_per_p[j]) != -1);

    p_w[i] -= tmp;
  }


  int* scmv_ = malloc1<int>(5 * max_num_rec, "todo");

  int* move_lr = scmv_;
  int* idx_v = scmv_ + max_num_rec;
  int* sc_to_l = scmv_ + max_num_rec*2;
  int* sc_to_r = scmv_ + max_num_rec*3;
  int* chg_lr_p_w = scmv_ + max_num_rec*4;
  int* chg_my_p_w = sc_to_r;

  void* pq_space = malloc_b((sizeof(idx_sc_pair<int>) + sizeof(int)) * (max_num_rec + 1), "todo324");
  priority_queue1 pq(pq_space, max_num_rec);

  max_num_rec = 0;

  for (i = 0; i < num_p; i++)
  {
    if (p_w[i] <= GLOBALS::blk_writable && p_num_v[i] <= GLOBALS::max_rec_p_page)
    {
      if (p_num_v[i] > max_num_rec)
        max_num_rec = p_num_v[i];
      continue;
    }
    
    //need to act
    my_p_w = p_w[i];
    my_num_v = p_num_v[i];

    if (i == 0)
    {
      l_p_w = GLOBALS::blk_writable + 1;
      l_num_v = GLOBALS::max_rec_p_page + 1;
    }
    else
    {
      l_p_w = p_w[i-1];
      l_num_v = p_num_v[i-1];
    }

    if (i == num_p - 1)
    {
      r_p_w = GLOBALS::blk_writable + 1;
      r_num_v = GLOBALS::max_rec_p_page + 1;
    }
    else
    {
      r_p_w = p_w[i+1];
      r_num_v = p_num_v[i+1];
    }

    if (     (my_p_w <= GLOBALS::blk_writable || 3*GLOBALS::blk_writable - my_p_w - l_p_w - r_p_w >= 0)
          && (my_num_v <= GLOBALS::max_rec_p_page || 3*GLOBALS::max_rec_p_page - my_num_v - l_num_v - r_num_v >= 0) )
    {
      // potential to give vertices to neighbors
      arr_set(move_lr, 0, my_num_v);
      arr_set(sc_to_l, 0, my_num_v);
      arr_set(sc_to_r, 0, my_num_v);
        
      j = v_per_p_begin[i];
      k = 0;
      do 
      {
        idx_v[k] = j;
        v_map[j] = k++;
      }
      while ((j = v_per_p[j]) != -1);
        
      cnt_v = p_num_v[i];
        
      for(;;)
      {
        max_to_l.set(-1, INT_MIN);
        max_to_r.set(-1, INT_MIN);
        
        for(j = 0; j < cnt_v; j++)
        {
          if (move_lr[j] == 0)
          {
            for (m = offset_e[idx_v[j]]; m < offset_e[idx_v[j]+1]; m++)
            {
              tmp = part[e_to[m]];
              if (tmp == i + p_add)
              {
                sc_to_l[j] -= e_w[m];
                sc_to_r[j] -= e_w[m];
              }
              else if (tmp == i + p_add - 1)
                sc_to_l[j] += e_w[m];
              else if (tmp == i + p_add + 1)
                sc_to_r[j] += e_w[m];
            }

            max_to_l.if_gr_set(j, sc_to_l[j]);
            max_to_r.if_gr_set(j, sc_to_r[j]);
          }
        } 

        if (max_to_l.idx != -1 && l_p_w < GLOBALS::blk_writable && l_num_v < GLOBALS::max_rec_p_page)
        {
          //potential to move to left (in general move left is preferable to move right, because not needed again)
          best_v = idx_v[max_to_l.idx];
          new_my_p_w = new_lr_p_w = v_w[best_v];
            
          for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
          {
            if (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] != -1)
              new_my_p_w -= e_w[j] * ee_ie_diff;
            else if (part[e_to[j]] == i + p_add - 1 || (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] == -1))
              new_lr_p_w -= e_w[j] * ee_ie_diff;
          }

          if (l_p_w + new_lr_p_w <= GLOBALS::blk_writable)
          {
            my_num_v--;
            my_p_w -= new_my_p_w;
            l_num_v++;
            l_p_w += new_lr_p_w;
            move_lr[max_to_l.idx] = -1;
            chg_lr_p_w[max_to_l.idx] = new_lr_p_w;
            l_go = true;
          }
          else
            l_go = false;
        }
        else
          l_go = false;

        if (my_p_w <= GLOBALS::blk_writable && my_num_v <= GLOBALS::max_rec_p_page)
        {
          success = true;
          break;
        }

        if (max_to_r.idx != -1 && r_p_w < GLOBALS::blk_writable 
            && r_num_v < GLOBALS::max_rec_p_page && move_lr[max_to_r.idx] == 0)
        {
          best_v = idx_v[max_to_r.idx];
          new_my_p_w = new_lr_p_w = v_w[best_v];

          for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
          {
            if (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] != 1)
              new_my_p_w -= e_w[j] * ee_ie_diff;
            else if (part[e_to[j]] == i + p_add + 1 || (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] == 1))
              new_lr_p_w -= e_w[j] * ee_ie_diff;
          }

          if (r_p_w + new_lr_p_w <= GLOBALS::blk_writable)
          {
            my_num_v--;
            my_p_w -= new_my_p_w;
            r_num_v++;
            r_p_w += new_lr_p_w;
            move_lr[max_to_r.idx] = 1;
            chg_lr_p_w[max_to_r.idx] = new_lr_p_w;
            r_go = true;
          }
          else
            r_go = false;
        }
        else
          r_go = false;

        if (my_p_w <= GLOBALS::blk_writable && my_num_v <= GLOBALS::max_rec_p_page)
        {
          success = true;
          break;
        } 
        else if (!l_go && !r_go)
        {
          success = false;
          break;
        }
      }

      ///////////////////////////////////////////
        
      if (success)
      {
        for (j = 0; j < cnt_v; j++)
          if (move_lr[j] != 0)
          {
            best_v = idx_v[j];
            new_p = i + move_lr[j];
            //do not need to change 
              
            part[best_v] = new_p + p_add;
            p_num_v[new_p]++;
            p_num_v[i]--;
          }
        
        p_num_v[i] = my_num_v;
        p_w[i] = my_p_w;
        
        /*if (i != 0) // not needed
        {
        p_num_v[i-1] = l_num_v;
        p_w[i-1] = l_p_w;
        }*/
        if (i != num_p-1)
        {
          p_num_v[i+1] = r_num_v;
          p_w[i+1] = r_p_w;
        }

        if_gr_set(my_num_v, max_num_rec);
        if_gr_set(l_num_v, max_num_rec);
        if_gr_set(r_num_v, max_num_rec);
      }

    } //large if loop
    else
      success = false;
      
    //////////////////////////////////////////////////////////////////////////

    if (!success)
    {
      // otherwise: split partition
      j = v_per_p_begin[i];
      k = cnt_new_p = 0;
      my_p_w = p_w[i];
      my_num_v = cnt_v = p_num_v[i];
      max_to_l.set(-1, INT_MAX);
      arr_set(move_lr, -1, my_num_v);
      arr_set(sc_to_l, 0, my_num_v);

      do 
      {
        assert(part[j] == i + p_add);

        tmp = 0;
        for (m = offset_e[j]; m < offset_e[j+1]; m++)
          tmp += e_w[j] * (part[e_to[m]] - (i + p_add));
          
        max_to_l.if_sm_set(k, tmp);
        idx_v[k] = j;
        v_map[j] = k++;

      } while ((j = v_per_p[j]) != -1);

      best_v = idx_v[max_to_l.idx];
      move_lr[max_to_l.idx] = cnt_new_p;

      l_p_w = v_w[best_v];
      my_p_w -= v_w[best_v];
      l_num_v = 1;
      my_num_v -= 1;
      
      for (j = offset_e[best_v]; j < offset_e[best_v + 1]; j++)
        if (part[e_to[j]] == i + p_add)
        {
          sc_to_l[v_map[e_to[j]]] += e_w[j];
          my_p_w += ee_ie_diff * e_w[j];
        }


      for(;;)
      {
        pq.reset();

        for (j = 0; j < cnt_v; j++)
          if (move_lr[j] == -1)
          {
            tmp = idx_v[j];
            new_lr_p_w = new_my_p_w = v_w[tmp];

            for (k = offset_e[tmp]; k < offset_e[tmp + 1]; k++)
              if (part[e_to[k]] == i + p_add)
                if (move_lr[v_map[e_to[k]]] == cnt_new_p)
                  new_lr_p_w -= ee_ie_diff * e_w[k];
                else if (move_lr[v_map[e_to[k]]] == -1)
                  new_my_p_w -= ee_ie_diff * e_w[k];

            chg_lr_p_w[j] = new_lr_p_w;
            chg_my_p_w[j] = new_my_p_w;

            pq.insert(j, sc_to_l[j]);
          }

        if (pq.empty()) //todo: maybe stop with new_my_p_w==0
          break;
        
        l_go = false;
        do 
        {
          max_to_l = pq.pop_max();

          if (l_p_w + chg_lr_p_w[max_to_l.idx] <= GLOBALS::blk_writable)
          { 
            l_go = true;
            break;
          }
        } while (!pq.empty());
        
        if (l_go)
        {
          move_lr[max_to_l.idx] = cnt_new_p;
          l_p_w += chg_lr_p_w[max_to_l.idx];
          my_p_w -= chg_my_p_w[max_to_l.idx];
          my_num_v--;
          l_num_v++;

          best_v = idx_v[max_to_l.idx];
          for (j = offset_e[best_v]; j < offset_e[best_v+1]; j++)
            if (part[e_to[j]] == i + p_add && move_lr[v_map[e_to[j]]] == -1)
              sc_to_l[v_map[e_to[j]]] += e_w[j]; 

          if (l_num_v >= GLOBALS::max_rec_p_page)
          {
            if_gr_set(l_num_v, max_num_rec);
            cnt_new_p++;
            l_num_v = 0;
            l_p_w = 0;
          }
        }
        else
        {
          if_gr_set(l_num_v, max_num_rec);
          cnt_new_p++;
          l_num_v = 0;
          l_p_w = 0;
        }
      }

      //assert (cnt_new_p >= 1);
      if (cnt_new_p == 0)
        print_endln();
      assert (my_p_w == 0);

      for (j = 0; j < num_v; j++)
        if (part[j] > i + p_add)
          part[j] += cnt_new_p;

      for (j = 0; j < cnt_v; j++)
      {
        assert(move_lr[j] != -1);
          
        part[idx_v[j]] = i + p_add + move_lr[j];
        p_w[i] = l_p_w;         // from last run
        p_num_v[i] = l_num_v;
        if_gr_set(l_num_v, max_num_rec);
      }
      p_add += cnt_new_p;
    }
  } // for each p

  if (FIRST_RUN)
  {
    max_num_rec = int(max_num_rec * 1.05); 

    if (max_num_rec == GLOBALS::blk_max_num_rec)
      GLOBALS::max_rec_p_page = GLOBALS::blk_max_num_rec;
    else
    {
      tmp = 0;
      while (max_num_rec >>= 1) ++tmp;
      tmp = 1 << tmp;
      if (tmp != max_num_rec)
        tmp <<= 1;
      GLOBALS::max_rec_p_page = tmp;
      if_sm_set(GLOBALS::blk_max_num_rec, GLOBALS::max_rec_p_page);
    }

    blk_init_fkts();
    FIRST_RUN = FALSE;
    GLOBALS::blk_var_start = 4 + GLOBALS::header_slt_len + 2* GLOBALS::ie_size;
  }
  
  num_p += p_add;
  graph->num_p = num_p;

  //required for printing statistics
  free_all(6, p_num_v, v_map, v_per_p, v_per_p_begin, scmv_, p_w);

  v_per_p = malloc1_set<int>(num_v, -1, "todo12");          //freed outside
  v_per_p_begin = malloc1<int>(num_p, "todo12");            //freed outside
  int* v_per_p_end = malloc1_set<int>(num_p, -1,"todo12");
  p_w = graph->p_w = malloc1_set<int>(num_p, 0,"todo12");   //part of graph - freed outside
  p_num_v = malloc1_set<int>(num_p, 0, "todo");

  for (i = 0; i < num_v; i++)
  {
    tmp = part[i];
    p_w[tmp] += v_w[i];
    p_num_v[tmp]++;

    if (v_per_p_end[tmp] == -1)
      v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
    else
    {
      v_per_p[v_per_p_end[tmp]] = i;
      v_per_p_end[tmp] = i;
    }
  }

  free(v_per_p_end);

  /*
  for (i = 0; i < num_p; i++)
  {
    tmp = 0;
    j = v_per_p_begin[i];

    do 
    {
      tmp2 = 0;
      for (k = offset_e[j]; k < offset_e[j] + org_size[j]; k++)
        if (part[e_to[k]] == i)
          tmp2 += ee_ie_diff;

      tmp += tmp2;
    } while ((j = v_per_p[j]) != -1);

    p_w[i] -= tmp;
  }
  */
  //light version, if not printing statistics
  //  free_all(3, p_num_v, v_map, scmv_, p_w);
  //  graph->p_w = NULL;

  // Combine partitions next to each other
  new_p = 0;
  for (i = 0; i < num_p; i++)
  {
    l_p_w = p_w[i];
    for (j = i + 1; j < num_p; j++)
    {
      my_p_w = my_num_v = tmp = 0;  
      for (k = i; k <= j; k++)
      {
        my_p_w += p_w[k];
        my_num_v += p_num_v[k];      
        m = v_per_p_begin[k];
        do 
        {
          tmp2 = 0;
          for (n = offset_e[m]; n < offset_e[m] + org_size[m]; n++)
            if (part[e_to[n]] >= i && part[e_to[n]] <= j)
              tmp2 += ee_ie_diff;

          tmp += tmp2;
        } while ((m = v_per_p[m]) != -1);
      }
      
      if (my_p_w - tmp > GLOBALS::blk_writable || my_num_v > GLOBALS::max_rec_p_page)
        break;
      l_p_w = my_p_w - tmp;
    }
    combine_p_lists(i, j-1, new_p, v_per_p, v_per_p_begin);
    p_w[new_p] = l_p_w;

    for (k = i; k < j; k++)
    {
      m = v_per_p_begin[k];
      do 
        part[m] = new_p;
      while ((m = v_per_p[m]) != -1);
    }
    new_p++;
    i = j - 1;
    if (new_p == 996)
      print_endln();
  }
  
  num_p = new_p;
  graph->num_p = num_p;
  
  free(p_num_v);
  v_per_p_begin = realloc1<int>(v_per_p_begin, num_p, "todo12", 1);
  p_w = graph->p_w = realloc1<int>(p_w, num_p, "todo12", 1);
}

