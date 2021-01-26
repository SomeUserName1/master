#include "query.h"

/* Simple SELECT ... WHERE ... query */
//#include <g-store.h>

q_vertex::q_vertex() : bm1(QUERY::buffer_memory / GLOBALS::blk_size, false)
{}

q_vertex::~q_vertex()
{}

void q_vertex::start()
{
  block1 b;
  header1 h;
  bool done = false;
  b_id cur_bid_start;
  int num;
  b_sc rec_in_b;
  int i, j;
  uint4 row_num = 1;
  g_id first_gid;
  int dummy;

  print_query_header();

  if (row_num >= QUERY::max_rownum)
    goto out_of_loop;

  for (cur_bid_start = 0; cur_bid_start < GLOBALS::cnt_blks; cur_bid_start += num)
  {
    bm1.fill_cons_from_start_no_map(b, cur_bid_start, num);
    
    for (i = 0; i < num; i++, b += GLOBALS::blk_size)
    {
      rec_in_b = blk_get_max_slt(b);
      first_gid = (cur_bid_start + i) * GLOBALS::max_rec_p_page;

      for (j = 0; j < rec_in_b; j++)
      {
        h = blk_get_header(b,j);
        if (get_header(h) == 0)      // invalid header
          continue;

        if (QUERY::pred_tree_where->evaluate(b, h, first_gid | j, row_num))
        {
          print_query_rec_new(b, h, row_num++, first_gid | j, 0, dummy, false);
          if (row_num >= QUERY::max_rownum)
            goto out_of_loop;
        }
      }
    } 
  }

out_of_loop:
  if (!QUERY::must_close_output)
    printf("\n");
  printf("Found %u records.\n\n", row_num - 1);
}