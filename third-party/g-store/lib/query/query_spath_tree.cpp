#include "query.h"

/* See dissertation Section 7.6 */
//#include <g-store.h>

//TODO: check logic behind start with ROWNUM here
void q_spath_tree::print_query_rec_new_wrap(block1 b, header1 h, b_id my_bid, b_sc my_slt, g_id my_gid, int lvl, uint4 my_start_v)
{
  bool iscycle = false;
  int dummy;
  
  QUERY::sel_root_arr = sel_root_arrs[my_start_v];
  
  /*if ((QUERY::sel_path = sel_paths[my_start_v].str) != NULL)
    add_to_sel_path(b, h, sel_paths[my_start_v].len);*/

  // TODO: possible optimization: differentiate between sel_iscycle and where_iscycle
  if (QUERY::sel_iscycle)
  {
    char* ie_list_begin = b + get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off;  
    char* ie_list_end = b + get_header(h + GLOBALS::ee_in_h);
    char* ee_list_end = b + get_header(h + GLOBALS::ee_end_in_h); 
    b_id bid;
    b_sc slt;
    g_id gid;
    uint4* tmp_uint4_ptr = v_found_paths + p_v_offset[my_bid] + my_slt;

    if (*tmp_uint4_ptr != PATH_FROM_GID)
      do
      {
        tmp_uint4_ptr = v_found_paths + *tmp_uint4_ptr;
        bid = get_lower_idx_n(uint4(tmp_uint4_ptr - v_found_paths), p_v_offset, GLOBALS::num_parts);
        slt = tmp_uint4_ptr - (v_found_paths + p_v_offset[bid]);
        gid = get_first_gid(bid) | slt;
        
        if (bid == my_bid)
        {
          if ((ie_bs(slt, ie_list_begin, ie_list_end)) != NULL)
          {
            iscycle = true;
            break;
          }
        }
        else
        {
          if ((ee_bs(gid, ee_list_end, ee_list_end)) != NULL)
          {
            iscycle = true;
            break;
          }
        }
      } while (*tmp_uint4_ptr != PATH_FROM_GID);
  }
  
  if (QUERY::pred_tree_where->evaluate(b, h, my_gid, row_num, lvl, iscycle))
    print_query_rec_new(b, h, row_num++, my_gid, lvl, dummy, iscycle);
};

void q_spath_tree::handle_start_with_v(block1 b, header1 h)
{
  char* str1;
  int i;
  char** tmp_sel_root_arr;

  /*if (QUERY::sel_path != NULL)
  {
    if (cnt_start_v > 0)
      sel_paths.push_back(str_len_pair(malloc1<char>(MAXLINE,"todo"), 0));
  }*/
  
  if (QUERY::cnt_sel_root > 0)
  {
    if (cnt_start_v > 0)
    {
      tmp_sel_root_arr = malloc1<char*>(QUERY::cnt_sel_root, "todo");
      tmp_sel_root_arr[0] = malloc1<char>(QUERY::cnt_sel_root * GLOBALS::blk_writable, "todo");
      for (i = 1; i < QUERY::cnt_sel_root; i++)
        tmp_sel_root_arr[i] = tmp_sel_root_arr[0] + GLOBALS::blk_writable * i;

      sel_root_arrs.push_back(tmp_sel_root_arr);
    } 
    else
      tmp_sel_root_arr = QUERY::sel_root_arr;

    for (i = 0; i < QUERY::select_list_len; i++)
      if (QUERY::select_list[i].field_datatype == sel_root_)
      {
        str1 = tmp_sel_root_arr[QUERY::select_list[i].sel_char];
        (QUERY::select_list[i].*(QUERY::select_list[i].eff_append_to_str))(str1, b, h);
        *str1 = '\0';
      }
  }
}


q_spath_tree::q_spath_tree() : bm1(QUERY::buffer_memory / GLOBALS::blk_size, true), bm3(QUERY::traverse_memory / sizeof(bv_list_slt_start_with))
{
  p_bv_list_begin_cur = new bv_list_begin<bv_list_slt_start_with>[GLOBALS::num_parts];
  p_bv_list_begin_nxt = new bv_list_begin<bv_list_slt_start_with>[GLOBALS::num_parts];
  p_v_offset = malloc1<g_id>(GLOBALS::num_parts, "todo");
  v_found_paths = malloc1_set<g_id>(GLOBALS::num_vertices, PATH_NOT_SEEN, "todo");

  //sel_paths.push_back(str_len_pair(QUERY::sel_path, 0));
  sel_root_arrs.push_back(QUERY::sel_root_arr);

  char* str1 = GLOBALS::v_p_blk;
  p_v_offset[0] = 0;
  for (int i = 1; i < GLOBALS::num_parts; i++)
    p_v_offset[i] = p_v_offset[i-1] + get_ie_adv(str1);

  row_num = 1;
  cnt_start_v = 0;
}

q_spath_tree::~q_spath_tree()
{
  delete [] p_bv_list_begin_cur;
  delete [] p_bv_list_begin_nxt;
  free(p_v_offset);
  free(v_found_paths);
  if (QUERY::must_close_output)
    fclose(QUERY::output_fp);
  //todo: free vectors
}

void q_spath_tree::start()
{
  block1 b;
  header1 h;

  b_id cur_bid_start, cur_bid;
  bool found_from = false , found_to = false;
  int num;
  b_sc num_max_rec;
  int i, j;
  bool did_sth;
  bv_list_slt_start_with* p_bv_list_e_ptr, *pbvl_ep2;
  bv_list_slt_start_with p_bv_list_e;
  int cnt_lvl = 0;
  g_id first_gid;
  uint4* tmp_uint4_ptr;

  print_query_header();

  block1 block_to = malloc1<char>(GLOBALS::blk_size,"todo");

  for (cur_bid_start = 0; cur_bid_start < GLOBALS::cnt_blks; cur_bid_start += num)
  {
    bm1.fill_cons_from_start(b, cur_bid_start, num);

    for (i = 0, cur_bid = cur_bid_start; i < num; i++, cur_bid++, bm1.get_next_blk(b))
    {
      num_max_rec = blk_get_max_slt(b);
      first_gid = cur_bid * GLOBALS::max_rec_p_page;

      for (j = 0; j < num_max_rec; j++)
      {
        h = blk_get_header(b, j);
        if (get_header(h) == 0)      // invalid header
          continue;

        tmp_uint4_ptr = v_found_paths + p_v_offset[cur_bid] + j;

        // may be set in if two vertices that satisfy START WITH criteria are neighbors
        if (*tmp_uint4_ptr != PATH_FROM_GID)
          if (QUERY::pred_tree_start_with->evaluate(b, h, first_gid | j, row_num, 0, false))
          { 
            // possible relict if multiple vertices that satisfy START WITH criteria are neighbors
            // TODO: maybe in nxt?
            if (*tmp_uint4_ptr != PATH_NOT_SEEN)
            {
              p_bv_list_e_ptr = p_bv_list_begin_cur[cur_bid].begin_v;
              if (p_bv_list_e_ptr->slt == j)
              {
                p_bv_list_begin_cur[cur_bid].begin_v = p_bv_list_e_ptr->next;
                bm3.give_back(p_bv_list_e_ptr);
              }
              else
              {
                pbvl_ep2 = p_bv_list_e_ptr->next;
                while (pbvl_ep2->slt != j)
                {
                  pbvl_ep2 = pbvl_ep2->next;
                  p_bv_list_e_ptr  = p_bv_list_e_ptr->next;
                }
                p_bv_list_e_ptr->next = pbvl_ep2->next;
                bm3.give_back(pbvl_ep2);
              }
            }

            *tmp_uint4_ptr = PATH_FROM_GID;
            
            handle_start_with_v(b, h);
            print_query_rec_new_wrap(b, h, cur_bid, j, first_gid | j, 0, cnt_start_v);
            traverse(b, h, cur_bid, j, true, cnt_start_v++);
          }
          else if (*tmp_uint4_ptr == PATH_NOT_SEEN && !QUERY::pred_tree_through->evaluate(b, h, first_gid | j))
            *tmp_uint4_ptr = PATH_UNFIT;
      }

      //////////////////////////////////////////////////////////////////////////

      while ((p_bv_list_e_ptr = p_bv_list_begin_cur[cur_bid].begin_v) != NULL)
      {
        p_bv_list_e = *p_bv_list_e_ptr;
        bm3.give_back(p_bv_list_e_ptr);
        p_bv_list_begin_cur[cur_bid].begin_v = p_bv_list_e.next;

        h = blk_get_header(b, p_bv_list_e.slt);

        // forward it, or not
        if (QUERY::pred_tree_through->evaluate(b, h, first_gid | p_bv_list_e.slt))
          p_bv_list_begin_nxt[cur_bid].add(bm3.put(bv_list_slt_start_with(p_bv_list_e.slt, p_bv_list_e.start_with_v_num)));
        else
          v_found_paths[p_v_offset[cur_bid] + p_bv_list_e.slt] = PATH_UNFIT;
      }
    } 
  }

  for (i = 0; i < GLOBALS::cnt_blks; i++)
  {
    while ((p_bv_list_e_ptr = p_bv_list_begin_cur[i].begin_v) != NULL)
    {    
      p_bv_list_e = *p_bv_list_e_ptr;
      bm3.give_back(p_bv_list_e_ptr);
      p_bv_list_begin_cur[i].begin_v = p_bv_list_e.next;

      // these have been checked already against through criteria
      p_bv_list_begin_nxt[i].add(bm3.put(bv_list_slt_start_with(p_bv_list_e.slt, p_bv_list_e.start_with_v_num)));
    }
  }

  //p_bv_list_begin_cur is now empty, p_bv_list_begin_nxt contains nodes on lvl 1

  //////////////////////////////////////////////////////////////////////////

  do
  {
    swap1(p_bv_list_begin_cur, p_bv_list_begin_nxt);
    did_sth = false;
    cnt_lvl++;

    while (bm1.fill_non_cons_bv_list(b, p_bv_list_begin_cur, num))  //fills b
    {
      did_sth = true;

      for (i = 0; i < num; i++, bm1.get_next_blk(b))  //fills b
      {
        cur_bid = get_block_id(blk_get_first_gid(b));

        while ((p_bv_list_e_ptr = p_bv_list_begin_cur[cur_bid].begin_v) != NULL)
        {
          p_bv_list_e = *p_bv_list_e_ptr;
          bm3.give_back(p_bv_list_e_ptr);
          p_bv_list_begin_cur[cur_bid].begin_v = p_bv_list_e.next;
          h = blk_get_header(b, p_bv_list_e.slt);
          print_query_rec_new_wrap(b, h, cur_bid, p_bv_list_e.slt, first_gid | j, cnt_lvl, p_bv_list_e.start_with_v_num);

          traverse(b, h, cur_bid, p_bv_list_e.slt, false, p_bv_list_e.start_with_v_num);
        }
      }
    }     
  } while (did_sth);

  print_ln("Longest path: %d. Found %u records.\n\n", cnt_lvl-1, row_num-1);
}

void q_spath_tree::traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0, uint4 my_start_v_cnt)
{
  b_sc ie;
  g_id ee;
  b_id ee_bid;
  b_sc ee_slt;
  char* str1 = b + get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off;  
  char* ie_list_end = b + get_header(h + GLOBALS::ee_in_h);
  char* ee_list_end = b + get_header(h + GLOBALS::ee_end_in_h);
  uint4* tmp_uint4_ptr;
  g_id first_gid = get_first_gid(my_bid);

  while (str1 < ie_list_end)
  {
    ie = get_ie_adv(str1);

    tmp_uint4_ptr = v_found_paths + p_v_offset[my_bid] + ie;
    switch (*tmp_uint4_ptr)
    {
    case PATH_NOT_SEEN:
      // RUN0
      if (run0) 
      {
        h = blk_get_header(b, ie);
        assert (get_header(h) != 0);

        if (QUERY::pred_tree_start_with->evaluate(b, h, first_gid | ie, row_num, 0, false))
        {
          *tmp_uint4_ptr = PATH_FROM_GID;
          handle_start_with_v(b, h);
          print_query_rec_new_wrap(b, h, my_bid, ie, first_gid | ie, 1, cnt_start_v);
          traverse(b, h, my_bid, ie, true, cnt_start_v++);
        } 
        else if (QUERY::pred_tree_through->evaluate(b, h, first_gid | ie))
        { 
          *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
          p_bv_list_begin_nxt[my_bid].add(bm3.put(bv_list_slt_start_with(ie, my_start_v_cnt)));
        }
        else
          *tmp_uint4_ptr = PATH_UNFIT;
      }
      // RUN1+
      else
      {
        *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
        p_bv_list_begin_nxt[my_bid].add(bm3.put(bv_list_slt_start_with(ie, my_start_v_cnt)));
      }
      break;

    case PATH_FROM_GID:
    case PATH_UNFIT:
    default:
      //end this path here
      break; 
    }
  }

  //////////////////////////////////////////////////////////////////////////

  str1 = ie_list_end;  

  while (str1 < ee_list_end)
  {
    ee = get_ee_adv(str1);
    ee_bid = get_block_id(ee);
    ee_slt = get_slot(ee);

    tmp_uint4_ptr = v_found_paths + p_v_offset[ee_bid] + ee_slt;

    switch (*tmp_uint4_ptr)
    {
    case PATH_NOT_SEEN:
      if (run0)
        if (!bm1.in_mem(b, ee_bid))  //this sets b
        {
          // node is tested or thrown out later
          *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
          p_bv_list_begin_cur[ee_bid].add(bm3.put(bv_list_slt_start_with(ee_slt, my_start_v_cnt)));
          break;
        }
        else
        {
          h = blk_get_header(b, ee_slt);
          assert (get_header(h) != 0);

          if (QUERY::pred_tree_start_with->evaluate(b, h, ee, row_num, 0, false))
          {
            *tmp_uint4_ptr = PATH_FROM_GID;
            handle_start_with_v(b, h);
            print_query_rec_new_wrap(b, h, ee_bid, ee_slt, ee, 1, cnt_start_v);
            traverse(b, h, ee_bid, ee_slt, true, cnt_start_v++);
          }
          else if (QUERY::pred_tree_through->evaluate(b, h, ee))
          { 
            *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
            p_bv_list_begin_nxt[ee_bid].add(bm3.put(bv_list_slt_start_with(ee_slt, my_start_v_cnt)));
          }
          else
            *tmp_uint4_ptr = PATH_UNFIT;
        }
      else
      {
        *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
        p_bv_list_begin_nxt[ee_bid].add(bm3.put(bv_list_slt_start_with(ee_slt, my_start_v_cnt)));
      }
      break;

    case PATH_FROM_GID:
    case PATH_UNFIT:
    default:
      //end this path here
      break; 
    }
  }
}