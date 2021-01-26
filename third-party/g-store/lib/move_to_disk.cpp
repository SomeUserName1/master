#include "move_to_disk.h"

#include <stdio.h>

#include "util.h"
#include "util_t.h"
#include "structs.h"

/* See dissertation Section 6.2 */
//#include <g-store.h>

int move_graph_to_disk(int mode)
{
  graph1* graph_ptr = ORG_GRAPH; 

  if (mode == 1)
  { 
    if (graph_ptr->coarser == NULL)
      return -1;

    while (graph_ptr->on_disk != NULL)
    { 
      if (graph_ptr->coarser == NULL)
        return -1;

      graph_ptr = graph_ptr->coarser;
    }
  }
  else
  {
    if (graph_ptr->coarser == NULL || graph_ptr->coarser->coarser == NULL)
      return -1;
    
    while (graph_ptr->on_disk != NULL)
    { 
      if (graph_ptr->coarser->coarser == NULL)
        return -1;
      
      graph_ptr = graph_ptr->coarser;
    }
  }
    
  //print_ln("moving graph at c_level %d to disk...", graph_ptr->c_level);

  int num_v = graph_ptr->num_v;
  int num_e = graph_ptr->num_e;
  int* offset_e = graph_ptr->offset_e;
  int* v_w = graph_ptr->v_w;
  int* map_in_c = graph_ptr->map_in_c;
  int* e_to = graph_ptr->e_to;
  int* e_w = graph_ptr->e_w;
  
  assert(map_in_c != NULL && e_to != NULL && e_w != NULL && v_w != NULL && offset_e != NULL);
  
  FILE* on_disk = graph_ptr->on_disk = tmpfile();

  printf("Moving graph at level %d to disk...", graph_ptr->c_level);

  size_t cnt_write = fwrite(offset_e, sizeof(int), num_v, on_disk);
  cnt_write += fwrite(v_w, sizeof(int), num_v, on_disk);
  cnt_write += fwrite(map_in_c, sizeof(int), num_v, on_disk);
  cnt_write += fwrite(e_to, sizeof(int), num_e, on_disk);
  cnt_write += fwrite(e_w, sizeof(int), num_e, on_disk);
 
  print_ln("done", graph_ptr->c_level);

  if (cnt_write != 3 * num_v + 2 * num_e)
    error_exit("Cannot move graph at c_level %d (size: %d int) to disk: disk full?", graph_ptr->c_level, 3 * num_v + 2 * num_e);

  fseek(on_disk, 0, SEEK_SET);

  free_all(5, offset_e, v_w, map_in_c, e_to, e_w);
  
  graph_ptr->offset_e = graph_ptr->v_w = graph_ptr->map_in_c = graph_ptr->e_to = graph_ptr->e_w = NULL;

  return 0;
}

void read_graph_from_disk(graph1 *graph)
{
  int num_v = graph->num_v;
  int num_e = graph->num_e;
  FILE* on_disk = graph->on_disk;

  //print_ln("reading graph at c_level %d from disk...", graph->c_level);

  graph->offset_e = malloc1<int>(num_v, "read_graph_from_disk: offset_e");
  graph->v_w = malloc1<int>(num_v, "read_graph_from_disk: v_w");
  graph->map_in_c = malloc1<int>(num_v, "read_graph_from_disk: map_in_c");
  graph->e_to = malloc1<int>(num_e, "read_graph_from_disk: e_to");
  graph->e_w = malloc1<int>(num_e, "read_graph_from_disk: e_w");

  fread(graph->offset_e, sizeof(int), num_v, on_disk);
  fread(graph->v_w, sizeof(int), num_v, on_disk);
  fread(graph->map_in_c, sizeof(int), num_v, on_disk);
  fread(graph->e_to, sizeof(int), num_e, on_disk);
  fread(graph->e_w, sizeof(int), num_e, on_disk);
  
  fclose(on_disk);
}