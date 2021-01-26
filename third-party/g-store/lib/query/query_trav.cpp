#include "query.h"

/* See dissertation Section 7.5 */
//#include <g-store.h>

//TODO: deal with max_traverse_depth


q_trav::q_trav() : bm2((QUERY::buffer_memory / GLOBALS::blk_size) % 2 == 0 ?
    QUERY::buffer_memory / GLOBALS::blk_size : QUERY::buffer_memory / GLOBALS::blk_size - 1 )
    // this a quick fix to deal with the problem that indexed_dequeue works 
    // only with an even number of parts.
{
  trav_list = malloc1<g_id>(PARAM::max_traverse_depth, "todo");
  //cycle_list = malloc1<b_sc>(GLOBALS::blk_writable / GLOBALS::ie_size, "todo");
}

q_trav::~q_trav()
{
  free(trav_list);
}

void q_trav::start()
{
  block1 b;
  header1 h;
  b_sc rec_in_b;
  b_sc j;
  int sel_path_len = 0;
  uint4 row_num = 1;
  g_id first_gid;

  print_query_header();

  for (b_id i = 0; i < GLOBALS::cnt_blks; i++)
  {
    bm2.find_or_fill(b, i);
    
    rec_in_b = blk_get_max_slt(b);
    first_gid = i * GLOBALS::max_rec_p_page;

    for (j = 0; j < rec_in_b; j++)
    {
      h = blk_get_header(b, j);
      if (get_header(h) == 0)      // invalid header
        continue;

      if (QUERY::pred_tree_start_with->evaluate(b, h, first_gid | j, row_num, 0, false))
      {
        // get the text for root fields
        if (QUERY::cnt_sel_root > 0)
          add_to_sel_root_arr(b, h);

        bm2.start_traverse(i);

        if (traverse(b, h, i, j, row_num, 0, sel_path_len))
          goto out_of_loop;
      }
    } 
  }

out_of_loop:
  //if (QUERY::must_close_output)
  if (!QUERY::must_close_output)
    printf("\n");
  printf("Found %u records.\n\n", row_num - 1);
}

bool q_trav::traverse(block1 b, header1 h, b_id my_bid, b_sc my_slt, uint4 &row_num, int lvl, int sel_path_len)
{
  b_sc ie;
  g_id ee;
  b_id ee_bid;
  b_sc ee_slt;
  header1 he;
  block1 be;
  char* str1 = b + get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off;  
  char* ie_list_end = b + get_header(h + GLOBALS::ee_in_h);
  char* ee_list_end = b + get_header(h + GLOBALS::ee_end_in_h);
  bool is_cycle = false;
  g_id my_gid = get_first_gid(my_bid) | my_slt;
  int assumed_mem_slt = bm2.get_mem_slot_num(my_bid);

  for (int i = 0; i< lvl; i++)
    if (trav_list[i] == my_gid)
      is_cycle = true;

  if (QUERY::sel_path != NULL)
    add_to_sel_path(b, h, sel_path_len);

  if (QUERY::pred_tree_where->evaluate(b, h, my_gid, row_num, lvl, is_cycle))
    print_query_rec_new(b, h, row_num++, my_gid, lvl, sel_path_len, is_cycle);
  
  // that is what Oracele does
  /*if (!QUERY::nocycle && is_cycle)
  {
    push_on_output("QUERY STOPPED: Found cycle. (Use NOCYCLE to ignore cycles.)\n");
    if (QUERY::must_close_output)
      printf("QUERY STOPPED: Found cycle. (Use NOCYCLE to ignore cycles.)\n");

    return true;
  }*/
  
  if (QUERY::nocycle && is_cycle)
    return false;

  trav_list[lvl] = my_gid;
  //identify_cycles(str1, ie_list_end, ee_list_end, my_bid, lvl, num_cycles);
  //cycle_list[num_cycles] = UINT2_MAX;

  if (row_num >= QUERY::max_rownum)
    return true;

  if (lvl >= QUERY::last_lvl_q)
    return false;

  if (lvl >= PARAM::max_traverse_depth)
  {
    push_on_output("Warning: Maximum traverse depth (parameter max_traverse_depth) reached.\n");
    if (QUERY::must_close_output)
      printf("Warning: Maximum traverse depth (parameter max_traverse_depth) reached. "
      "Control depth using a predicate on LEVEL or change parameter.\n");
    return false;
  }

  while (str1 < ie_list_end)
  {
    if (bm2.confirm_block(my_bid, assumed_mem_slt, b, h, str1))
    {        
      // b, h, str1 are updated in the function
      ie_list_end = b + get_header(h + GLOBALS::ee_in_h);
      ee_list_end = b + get_header(h + GLOBALS::ee_end_in_h);
    }

    ie = get_ie_adv(str1);
    he = blk_get_header(b, ie);
    assert (get_header(he) != 0);
    bm2.pin(assumed_mem_slt);

    if (traverse(b, he, my_bid, ie, row_num, lvl + 1, sel_path_len))
      return true;
  }

  //////////////////////////////////////////////////////////////////////////
  
  while (str1 < ee_list_end)
  {
    if (bm2.confirm_block(my_bid, assumed_mem_slt, b, h, str1))
    {        
      // b, h, str1 are updated in the function
      ee_list_end = b + get_header(h + GLOBALS::ee_end_in_h);
    }

    ee = get_ee_adv(str1);
    ee_bid = get_block_id(ee);
    ee_slt = get_slot(ee);
      
    bm2.in_mem_or_add_pin(be, ee_bid);
    he = blk_get_header(be, ee_slt);
    assert (get_header(he) != 0);
    
    if (traverse(be, he, ee_bid, ee_slt, row_num, lvl + 1, sel_path_len))
      return true;
  }

  bm2.unpin(assumed_mem_slt);
  return false;
}

/*
void q_trav::identify_cycles(char* ie_list_begin, char* ie_list_end, char* ee_list_end, b_id my_bid, int lvl, b_sc &num_cycles)
{
  g_id gid;
  b_id bid;
  char* str1;
  int ie_len = (ie_list_end - ie_list_begin) / GLOBALS::ie_size;

  num_cycles = 0;

  for (int i = 0; i < lvl; i++)
  {
    gid = trav_list[i];
    bid = get_block_id(gid);
    if (my_bid == bid)
    {
      if ((str1 = ie_bs(get_slot(gid), ie_list_begin, ie_list_end)) != NULL)
        cycle_list[num_cycles++] = (str1 - ie_list_begin) / GLOBALS::ie_size;
    }
    else
    {
      if ((str1 = ee_bs(gid, ee_list_end, ee_list_end)) != NULL)
        cycle_list[num_cycles++] = (str1 - ie_list_end) / 4 + ie_len;
    }
  }

  qsort(cycle_list, num_cycles, sizeof(b_sc), bs_comp_int<b_sc>);
}*/