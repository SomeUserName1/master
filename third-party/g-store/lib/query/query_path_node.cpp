#include "query.h"

/* See dissertation Section 7.6 */
//#include <g-store.h>

q_path_node::q_path_node() : bm1(QUERY::buffer_memory / GLOBALS::blk_size, true), bm3(QUERY::traverse_memory / sizeof(bv_list_slt))
{
  p_bv_list_begin = new bv_list_begin<bv_list_slt>[GLOBALS::num_parts]; // 
  p_v_offset = malloc1<g_id>(GLOBALS::num_parts, "todo");
  
  char* str1 = GLOBALS::v_p_blk;
  p_v_offset[0] = 0;
  for (int i = 1; i < GLOBALS::num_parts; i++)
    p_v_offset[i] = p_v_offset[i-1] + get_ie_adv(str1);

  v_found_paths = malloc1_set<g_id>(GLOBALS::num_vertices, PATH_NOT_SEEN, "todo"); // malloc and sets entire array to PATH_NOT_SEEN
}

q_path_node::~q_path_node()
{
  delete [] p_bv_list_begin;
  free(p_v_offset);
  free(v_found_paths);
}

void q_path_node::start()
{
  block1 b; // retrieve stream of blocks, handle to position in stream. stream usually large number of blocks. points to any of the block
  header1 h; // used same principle as b
  bool found_from = false , found_to = false; // keep track of found entry fits to start/end conditions. for error messages. not used in code v much
  b_id cur_bid_start; // 
  b_id cur_bid; // tracking blocks cycling through disk
  int num; // increases cur_bid for each iteration; number of blocks that could be retrieved
  b_sc num_max_rec; // cycle each block, number of records
  int i, j;
  g_id first_gid;

  bv_list_slt* p_bv_list_e_ptr; // struct used to keep track of blocks to look at later
  bv_list_slt p_bv_list_e; // retrieve something from array; use as pointer into
  
  uint4* tmp_uint4_ptr; // stores, for each vertex, if seen/not seen. <=> pred

  for (cur_bid_start = 0; cur_bid_start < GLOBALS::cnt_blks; cur_bid_start += num) // all blocks on disk
  {
    bm1.fill_cons_from_start(b, cur_bid_start, num);

	for (i = 0, cur_bid = cur_bid_start; i < num; i++, cur_bid++, bm1.get_next_blk(b)) // cycle through blocks retrieved, in buffer
    {
      num_max_rec = blk_get_max_slt(b);
      first_gid = cur_bid * GLOBALS::max_rec_p_page;
      
      for (j = 0; j < num_max_rec; j++) // iterating through all records in current block
      {
        h = blk_get_header(b, j);
        if (get_header(h) == 0)      // invalid header
          continue;

        tmp_uint4_ptr = &v_found_paths[p_v_offset[cur_bid] + j]; // address of predicate of vertex/record

        if (QUERY::pred_tree_start_with->evaluate(b, h, first_gid | j, 1, 0, false))
        { 
          found_from = true;
          *tmp_uint4_ptr = PATH_FROM_GID; // mark all starting with PATH_FROM_GID number
          
          if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | j))
          {
            print_found_path(cur_bid, j, &bm1, &bm3, p_v_offset, v_found_paths);
            return;
          }

          if (traverse(b, h, cur_bid, j, true, 0))
            return;
        } 
        else if (*tmp_uint4_ptr == PATH_NOT_SEEN)
        {
          if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | j))
          {
            found_to = true;
            *tmp_uint4_ptr = PATH_TO_GID;
          }
        }
      }

      //////////////////////////////////////////////////////////////////////////
      
      while ((p_bv_list_e_ptr = p_bv_list_begin[cur_bid].begin_v) != NULL)
      {
        p_bv_list_e = *p_bv_list_e_ptr;
        bm3.give_back(p_bv_list_e_ptr); // give back element, can now put new item there
        p_bv_list_begin[cur_bid].begin_v = p_bv_list_e.next;

        h = blk_get_header(b, p_bv_list_e.slt);

        if (QUERY::pred_tree_end_with->evaluate(b, h, first_gid | p_bv_list_e.slt))
        {
          print_found_path(cur_bid, p_bv_list_e.slt, &bm1, &bm3, p_v_offset, v_found_paths);
          return;
        }

        if (QUERY::pred_tree_through->evaluate(b, h, first_gid | p_bv_list_e.slt))
        { 
          if (traverse(b, h, cur_bid, p_bv_list_e.slt, true, 0))
            return;
        }
        else
          v_found_paths[p_v_offset[cur_bid] + p_bv_list_e.slt] = PATH_UNFIT;
      }
    } 
  }

  //////////////////////////////////////////////////////////////////////////
  while (bm1.fill_non_cons_bv_list(b, p_bv_list_begin, num))  //fills b
  {
    for (i = 0; i < num; i++, bm1.get_next_blk(b))
    {
      first_gid = blk_get_first_gid(b);
      cur_bid = get_block_id(first_gid);
      
      while ((p_bv_list_e_ptr = p_bv_list_begin[cur_bid].begin_v) != NULL)
      {
        p_bv_list_e = *p_bv_list_e_ptr;
        bm3.give_back(p_bv_list_e_ptr);
        p_bv_list_begin[cur_bid].begin_v = p_bv_list_e.next;
        h = blk_get_header(b, p_bv_list_e.slt);

        if (QUERY::pred_tree_through->evaluate(b, h, first_gid | p_bv_list_e.slt))
        {                                                         
          if(traverse(b, blk_get_header(b, p_bv_list_e.slt), cur_bid, p_bv_list_e.slt, false, 0))
            return;
        }
        else
          v_found_paths[p_v_offset[cur_bid] + p_bv_list_e.slt] = PATH_UNFIT;
      }
    }
  }

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


//level only to stop from too large recursion
bool q_path_node::traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, bool run0, int lvl) // b, h position in buffer; my_bid, my_slot identifiers
{
  if (lvl > PARAM::max_traverse_depth) // capture stack overflow
  {
    p_bv_list_begin[my_bid].add(bm3.put(bv_list_slt(my_slt))); // p_bv_list_begin array of all bv_list's. bm3 manages the space and it's stored there, then p_bv_list stores the pointer to this space
    return false;
  }
  
  b_sc ie;
  g_id ee;
  b_id ee_bid;
  b_sc ee_slt;
  char* str1 = b + get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off; // temp variable cycle through int/ext edge lists
  char* ie_list_end = b + get_header(h + GLOBALS::ee_in_h); // internal edge list
  char* ee_list_end = b + get_header(h + GLOBALS::ee_end_in_h); // external edge list
  uint4* tmp_uint4_ptr;
  header1 he;
  block1 be;
  
  uint4 mymark = p_v_offset[my_bid] + my_slt; // at this point, neighbourhood of current vertex. also forms chain
  
  while (str1 < ie_list_end) // iterate internal edges
  {
    ie = get_ie_adv(str1); // slot number of vertex connected to current vertex (internal)
    
    tmp_uint4_ptr = &v_found_paths[p_v_offset[my_bid] + ie];
    
    switch (*tmp_uint4_ptr)
    {
    case PATH_NOT_SEEN:
      he = blk_get_header(b, ie); // do not need original header anymore
      assert (get_header(he) != 0);

      if (run0 && QUERY::pred_tree_end_with->evaluate(b, he, get_first_gid(my_bid) | ie))
      {
        *tmp_uint4_ptr = mymark;
        print_found_path(my_bid, ie, &bm1, &bm3, p_v_offset, v_found_paths);
        return true;
      }
      else if (QUERY::pred_tree_through->evaluate(b, he, get_first_gid(my_bid) | ie))
        *tmp_uint4_ptr = mymark; 
      else
        *tmp_uint4_ptr = PATH_UNFIT;

      break;

    case PATH_TO_GID:
      *tmp_uint4_ptr = mymark;
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

  while (str1 < ee_list_end) // neighbourhood of current vertex
  {
    ee = get_ee_adv(str1);
    ee_bid = get_block_id(ee);
    ee_slt = get_slot(ee);
        
    tmp_uint4_ptr = &v_found_paths[p_v_offset[ee_bid] + ee_slt];

    switch (*tmp_uint4_ptr)
    {
    case PATH_NOT_SEEN:
      if (!bm1.in_mem(be, ee_bid))  //this sets b
      {
        *tmp_uint4_ptr = mymark;
        p_bv_list_begin[ee_bid].add(bm3.put(bv_list_slt(ee_slt)));
      }        
      else
      {
        he = blk_get_header(be, ee_slt);
        assert (get_header(he) != 0);

        if (run0 && QUERY::pred_tree_end_with->evaluate(be, he, ee))
        {
          *tmp_uint4_ptr = mymark;
          print_found_path(ee_bid, ee_slt, &bm1, &bm3, p_v_offset, v_found_paths);
          return true;
        }
        else if (QUERY::pred_tree_through->evaluate(be, he, ee))
          *tmp_uint4_ptr = mymark;
        else
          *tmp_uint4_ptr = PATH_UNFIT;
      }
      break;

    case PATH_TO_GID:
      *tmp_uint4_ptr = mymark;
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

  str1 = b + get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off;  

  while (str1 < ie_list_end)
  {
    ie = get_ie_adv(str1);
    
    if (mymark == v_found_paths[p_v_offset[my_bid] + ie])
      if (traverse(b, blk_get_header(b, ie), my_bid, ie, run0, lvl + 1))
        return true; 
  }
  
  str1 = ie_list_end; 

  while (str1 < ee_list_end)
  {
    ee = get_ee_adv(str1);
    ee_bid = get_block_id(ee);
    ee_slt = get_slot(ee);

    if (mymark == v_found_paths[p_v_offset[ee_bid] + ee_slt]
        && bm1.in_mem(be, ee_bid))
      if (traverse(be, blk_get_header(be, ee_slt), ee_bid, ee_slt, run0, lvl+1))
        return true;
  }

  return false;
}
