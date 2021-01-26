#include "ml.h"

/* Coordinates the multilevel algorithm */
//#include <g-store.h>

void part_graph1(graph1 *graph)
{

  // TODO: MAKE VERTEXWEIGHT LONG INT FOR COARSER GRAPHS, maybe use template function :/
  
  graph1* graph_ptr = graph;
  
  //srand(uint4(time(0)));
  srand(1000);
    
  // coarsening 
  LARGE_INTEGER start, finish, freq;
  double t1, t2, t3, t4;

  QueryPerformanceFrequency(&freq);

  for (;;)
  {
    printf("Coarsen (lvl %03d): %08d vertices ", graph_ptr->c_level, graph_ptr->num_v);
  
    QueryPerformanceCounter(&start);
    
    if (coarsen(graph_ptr) == 1)
      break;
    
    QueryPerformanceCounter(&finish);
    printf("(%.2fs)\n", ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart));

    graph_ptr = graph_ptr->coarser;
  }
  
  QueryPerformanceCounter(&finish);
  printf("(%.2fs)\n", ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart));

  turn_around(graph_ptr);
  
  for (;;)
  {
    graph_ptr = graph_ptr->finer;
    
    if (uncoarsen(graph_ptr, t1, t2, t3, t4) == 1)
      break;
        
    printf("Uncoarsen (lvl %03d): %08d vertices in %05d parts ", graph_ptr->c_level, graph_ptr->num_v, graph_ptr->num_p);
    printf("(%.2fs %.2fs %.2fs)\n", t1, t2, t3);
  }

  printf("Uncoarsen (lvl %03d): %08d vertices in %05d parts ", graph_ptr->c_level, graph_ptr->num_v, graph_ptr->num_p);
  printf("(%.2fs %.2fs %.2fs %.2fs)\n", t1, t2, t3, t4);
  
} 
