#include "ml.h"

#include "util_t.h"
#include "util.h"

/* See dissertation Section 6.4 */
//#include <g-store.h>

void turn_around(graph1 *graph)
{
  int num_v = graph->num_v;
  int* v_w = graph->v_w;

  int* part = graph->part = malloc1<int>(num_v, "todo1", 1);
  int* p_w = malloc1_set<int>(num_v, 0, "todo1", 1);

  GLOBALS::max_c_level = graph->c_level;
  graph->num_p = num_v;

  int i, cnt_p = 0, cnt_v_p = 0;
  p_w[cnt_p] = 0;

  for (i = 0; i < num_v; i++)
  {
    if (cnt_v_p == 0 || p_w[cnt_p] + v_w[i] <= GLOBALS::blk_writable)   // TODO, do not use
    {
      p_w[cnt_p] += v_w[i];
      cnt_v_p++;
    }
    else
    {
      p_w[++cnt_p] = v_w[i];
      cnt_v_p = 0;
    }

    part[i] = cnt_p;
  }

  cnt_p++;
  graph->num_p = cnt_p;
  graph->p_w = realloc1<int>(p_w, cnt_p, "todo1", 1);

  GLOBALS::isle_map = malloc1<int>(cnt_p + 1, "todo1", 1);
  for (i = 0; i < cnt_p ; i++)
    GLOBALS::isle_map [i] = i;

  GLOBALS::isle_map[cnt_p] = -1;
}