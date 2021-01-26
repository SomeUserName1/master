#include "ml.h"

#include "move_to_disk.h"
#include "util_t.h"
#include "util.h"
#include "print_graph.h"
#include "evaluate.h"
#include "parameters.h"

/* See dissertation Section 6.5 */
//#include <g-store.h>

///////////////////////////////////////////////////////
//TODO: remove map in coarser early!

int uncoarsen(graph1 *graph, double &t1, double &t2, double &t3, double &t4)
{
  if (graph->on_disk != NULL)
    read_graph_from_disk(graph);

  graph1* c_graph = graph->coarser;

  int num_v     = graph->num_v;
  int* map_in_c = graph->map_in_c;
  int* c__part  = c_graph->part;
  int c__num_p  = c_graph->num_p;

  int i, tmp, max_p1, num_p;
  int *part;

  int* v_per_p = malloc1_set<int>(num_v, -1,"todo12", 2);
  int* v_per_p_begin = malloc1<int>(c__num_p, "todo12", 2);
  int* v_per_p_end = malloc1_set<int>(c__num_p, -1,"todo12", 2);

  std::vector<bool>* part_type = new std::vector<bool>;  
  
  LARGE_INTEGER start, finish, freq;
  QueryPerformanceFrequency(&freq);

  for (i = 0; i < num_v; i++)
  {
    tmp = c__part[map_in_c[i]];

    if (v_per_p_end[tmp] == -1)
      v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
    else
    {
      v_per_p[v_per_p_end[tmp]] = i;
      v_per_p_end[tmp] = i;
    }
  }

  free(v_per_p_end);
  
  QueryPerformanceCounter(&start);

  project(graph, v_per_p, v_per_p_begin, part_type, max_p1);

  QueryPerformanceCounter(&finish);
  t1 = ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);


  free_all(8, c_graph->offset_e, c_graph->v_w, c_graph->map_in_c, c_graph->e_to, 
              c_graph->e_w, c__part, c_graph->p_w, c_graph);
  
  graph->coarser = NULL;

  num_p = graph->num_p;
  part = graph->part;

  free(v_per_p_begin);
  
  arr_set<int>(v_per_p, -1, num_v);

  v_per_p_begin = malloc1<int>(num_p, "todo12", 1);
  v_per_p_end = malloc1_set<int>(num_p, -1,"todo12", 1);

  for (i = 0; i < num_v; i++)
  {
    tmp = part[i];
    if (v_per_p_end[tmp] == -1)
      v_per_p_begin[tmp] = v_per_p_end[tmp] = i;
    else
    {
      v_per_p[v_per_p_end[tmp]] = i;
      v_per_p_end[tmp] = i;
    }
  }

  free(v_per_p_end);

  //not needed until refine, but required for calls of evaluate
  graph->p_w  = malloc1_set<int>(num_p, 0, "todo756");         // todo: might have to be int8?
    
  QueryPerformanceCounter(&start);

  reorder(graph, v_per_p, v_per_p_begin, part_type, max_p1);

  QueryPerformanceCounter(&finish);
  t2 = ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);

  delete part_type;
  
  QueryPerformanceCounter(&start);

  refine(graph, v_per_p, v_per_p_begin);

  QueryPerformanceCounter(&finish);
  t3 = ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);
  
  if (graph->c_level == 0)
  {
    if (PARAM::debug_mode) 
    {
      print_graph3(graph,v_per_p,v_per_p_begin,"before finalization");
      evaluate(graph, v_per_p, v_per_p_begin,"before finalization");
    }
    
    QueryPerformanceCounter(&start);
    
    finalize(graph, v_per_p, v_per_p_begin);
    
    QueryPerformanceCounter(&finish);
    t4 = ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);

    if (PARAM::debug_mode) 
    {
      print_graph3(graph,v_per_p,v_per_p_begin,"after finalization");
    }
    
    evaluate(graph, v_per_p, v_per_p_begin, "after finalization");

    free_all(2, v_per_p, v_per_p_begin);
    return 1;
  }
  
  free_all(2, v_per_p, v_per_p_begin);
  return 0;
}
