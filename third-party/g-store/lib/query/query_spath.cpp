#include "query.h"

/* See dissertation Section 7.6 */
//#include <g-store.h>

q_spath::q_spath() : bm1(QUERY::buffer_memory / GLOBALS::blk_size, true), bm3(QUERY::traverse_memory / sizeof(bv_list_slt))
{
  p_bv_list_begin_cur = new bv_list_begin<bv_list_slt>[GLOBALS::num_parts];
  p_bv_list_begin_nxt = new bv_list_begin<bv_list_slt>[GLOBALS::num_parts];
  p_v_offset = malloc1<g_id>(GLOBALS::num_parts, "todo");
  v_found_paths = malloc1_set<g_id>(GLOBALS::num_vertices, PATH_NOT_SEEN, "todo");

  char* str1 = GLOBALS::v_p_blk;
  p_v_offset[0] = 0;
  for (int i = 1; i < GLOBALS::num_parts; i++)
    p_v_offset[i] = p_v_offset[i-1] + get_ie_adv(str1);
}

q_spath::~q_spath()
{
  delete [] p_bv_list_begin_cur;
  delete [] p_bv_list_begin_nxt;
  free(p_v_offset);
  free(v_found_paths);
}

void q_spath::start()
{
  block1 b;
  header1 h;

  b_id cur_bid_start, cur_bid;
  bool found_from = false , found_to = false;
  int num;
  b_sc num_max_rec;
  int i, j;
  bool did_sth;
  bv_list_slt* p_bv_list_e_ptr, *p_bv_list_e_ptr2;
  bv_list_slt p_bv_list_e;
  g_id first_gid;
  uint4* tmp_uint4_ptr;  

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

        tmp_uint4_ptr = &v_found_paths[p_v_offset[cur_bid] + j];

        // may be set if two vertices that satisfy START WITH criteria are neighbors
        if (*tmp_uint4_ptr != PATH_FROM_GID)
          if (QUERY::pred_tree_start_with->evaluate(b, h, first_gid | j, 1, 0, false))
          { 
            // possible relict if two vertices that satisfy START WITH criteria are neighbors
            if (*tmp_uint4_ptr != PATH_NOT_SEEN)
            {
              p_bv_list_e_ptr = p_bv_list_begin_nxt[cur_bid].begin_v;
              if (p_bv_list_e_ptr->slt == j)
              {
                p_bv_list_begin_nxt[cur_bid].begin_v = p_bv_list_e_ptr->next;
                bm3.give_back(p_bv_list_e_ptr);
              }
              else
              {
                p_bv_list_e_ptr2 = p_bv_list_e_ptr->next;
                while (p_bv_list_e_ptr2->slt != j)
                {
                  p_bv_list_e_ptr2 = p_bv_list_e_ptr2->next;
                  p_bv_list_e_ptr  = p_bv_list_e_ptr->next;
                }
                p_bv_list_e_ptr->next = p_bv_list_e_ptr2->next;
                bm3.give_back(p_bv_list_e_ptr2);
              }
            }

            *tmp_uint4_ptr = PATH_FROM_GID;
            found_from = true;
            if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | j))
            {
              print_found_path(cur_bid, j, &bm1, &bm3, p_v_offset, v_found_paths);
              return;
            }

            if (traverse(b, h, cur_bid, j, true))
              return;
          } 
          else if (*tmp_uint4_ptr == PATH_NOT_SEEN)
          {
            if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | j))
            {
              found_to = true;
              *tmp_uint4_ptr = PATH_TO_GID;
            }
            else if ( !QUERY::pred_tree_through->evaluate(b, h, first_gid | j))
              *tmp_uint4_ptr = PATH_UNFIT;
          }
      }

      //////////////////////////////////////////////////////////////////////////

      while ((p_bv_list_e_ptr = p_bv_list_begin_cur[cur_bid].begin_v) != NULL)
      {
        p_bv_list_e = *p_bv_list_e_ptr;
        bm3.give_back(p_bv_list_e_ptr);
        p_bv_list_begin_cur[cur_bid].begin_v = p_bv_list_e.next;

        // (now checked above) possible relict if two vertices that satisfy FROM criteria are neighbors
        //if (v_found_paths[p_v_offset[cur_bid] + p_bv_list_e.slt] == PATH_FROM_GID) 
          //continue;

        h = blk_get_header(b, p_bv_list_e.slt);

        if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | p_bv_list_e.slt))
        {
          print_found_path(cur_bid, p_bv_list_e.slt, &bm1, &bm3, p_v_offset, v_found_paths);
          return;
        }

        if (QUERY::pred_tree_through->evaluate(b, h, first_gid | p_bv_list_e.slt))
          p_bv_list_begin_nxt[cur_bid].add(bm3.put(bv_list_slt(p_bv_list_e.slt)));
        else
          v_found_paths[p_v_offset[cur_bid] + p_bv_list_e.slt] = PATH_UNFIT;
      }
    } 
  }

  //////////////////////////////////////////////////////////////////////////
  do
  {
    // swap guarantees no mixing of levels
    swap1(p_bv_list_begin_cur, p_bv_list_begin_nxt);
    did_sth = false;
      
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
 
          if (traverse(b, h, cur_bid, p_bv_list_e.slt, false))
            return;
        }
      }
    }
  } while (did_sth);

  push_on_output("No path exists.\n");
  if (QUERY::must_close_output)
    print_ln("No path exists.");

  if (!found_from)
  {
    push_on_output("No vertex matches START WITH criteria.\n");
    if (QUERY::must_close_output)
      print_ln("No vertex matches START WITH criteria.\n");
  }
  if (!found_to)
  {
    push_on_output("No vertex matches END WITH criteria.\n");
    if (QUERY::must_close_output)
      print_ln("No vertex matches END WITH criteria.\n");
  }

}

bool q_spath::traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0)
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

    tmp_uint4_ptr = &v_found_paths[p_v_offset[my_bid] + ie];
    switch (*tmp_uint4_ptr)
    {
    case PATH_NOT_SEEN:
      if (run0) 
      {
        h = blk_get_header(b, ie);
        assert (get_header(h) != 0);
        
        if (QUERY::pred_tree_start_with->evaluate(b, h, first_gid | ie, 1, 0, false))
        {
          *tmp_uint4_ptr = PATH_FROM_GID;
          if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | ie))
          {
            print_found_path(my_bid, ie, &bm1, &bm3, p_v_offset, v_found_paths);
            return true;
          }

          if (traverse(b, h, my_bid, ie, true))
            return true;
        } 
        else if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | ie))
        {
          *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
          print_found_path(my_bid, ie, &bm1, &bm3, p_v_offset, v_found_paths);
          return true;
        }
        else if (QUERY::pred_tree_through->evaluate(b, h, first_gid | ie))
        { 
          *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
          p_bv_list_begin_nxt[my_bid].add(bm3.put(bv_list_slt(ie)));
        }
        else
          *tmp_uint4_ptr = PATH_UNFIT;
      }
      else
      {
        *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
        p_bv_list_begin_nxt[my_bid].add(bm3.put(bv_list_slt(ie)));
      }
      break;

    case PATH_TO_GID:
      *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
      print_found_path(my_bid, ie, &bm1, &bm3, p_v_offset, v_found_paths);
      return true;
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

    tmp_uint4_ptr = &v_found_paths[p_v_offset[ee_bid] + ee_slt];

    switch (*tmp_uint4_ptr)
    {
    case PATH_NOT_SEEN:
      if (run0)
        if (!bm1.in_mem(b, ee_bid))  //this sets b
        {
          *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
          p_bv_list_begin_cur[ee_bid].add(bm3.put(bv_list_slt(ee_slt)));
          break;
        }
        else
        {
          h = blk_get_header(b, ee_slt);
          assert (get_header(h) != 0);

          if (QUERY::pred_tree_start_with->evaluate(b, h, ee, 1, 0, false))
          {
            *tmp_uint4_ptr = PATH_FROM_GID;
            if (QUERY::pred_tree_end_with->evaluate(b, h, ee))
            {
              print_found_path(ee_bid, ee_slt, &bm1, &bm3, p_v_offset, v_found_paths);
              return true;
            }

            if (traverse(b, h, ee_bid, ee_slt, true))
              return true;
          }
          else if (QUERY::pred_tree_end_with->evaluate(b, h, ee))
          {
            *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
            print_found_path(ee_bid, ee_slt, &bm1, &bm3, p_v_offset, v_found_paths);
            return true;
          }
          else if (QUERY::pred_tree_through->evaluate(b, h, ee))
          { 
            *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
            p_bv_list_begin_nxt[ee_bid].add(bm3.put(bv_list_slt(ee_slt)));
          }
          else
            *tmp_uint4_ptr = PATH_UNFIT;
        }
      else
      {
        *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
        p_bv_list_begin_nxt[ee_bid].add(bm3.put(bv_list_slt(ee_slt)));
      }
      break;

    case PATH_TO_GID:
      *tmp_uint4_ptr = p_v_offset[my_bid] + my_slt;
      print_found_path(ee_bid, ee_slt, &bm1, &bm3, p_v_offset, v_found_paths);
      return true;
      break;

    case PATH_FROM_GID:
    case PATH_UNFIT:
    default:
      //end this path here
      break; 
    }
  }

  return false;
}