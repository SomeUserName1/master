#include "entry_points.h"

#include <conio.h>

#include "util.h"
#include "util_t.h"
#include "read_input.h"
#include "multilevel/ml.h"
#include "print_graph.h"
#include "memory_mgr.h"
#include "read_to_blocks.h"
#include "parameters.h"
#include "evaluate.h"
#include "read_schema.h"
#include "calc_header.h"
#include "read_query.h"

/* Entry point functions for the storage algorithm. 
create_db_new() for a completely new representation with 
G-Store's multilevel algorithm.
create_db_part() based on a supplied partitioning file */
//#include <g-store.h>

graph1* ORG_GRAPH;

void create_db_new(FILE* fp_in)
{
  char filename[256];
  strcat1(filename, PARAM::working_dir, PARAM::default_parts_filename);
  FILE* fp_parts = fopen(filename, "w");

  if (fp_parts == NULL)
    error_exit("cannot create file '%s'", filename);
  
  strcat1(filename, PARAM::working_dir, PARAM::default_gidmap_filename);
  FILE* fp_gid_map = fopen(filename, "w+");
  
  if (fp_gid_map == NULL)
    error_exit("cannot create file '%s'", filename);

  gstore_init();

  graph1* org_graph = malloc_graph1_init("main_graph from create_db_new()");
  ORG_GRAPH = org_graph;

  read_graph(org_graph, fp_in);
  
  part_graph1(org_graph);

  print_part_gid(org_graph, fp_parts, fp_gid_map);

  mem_create_disks_blks(org_graph->num_p, PARAM::max_disk_size);

  free_graph1(org_graph);

  read_to_blocks(fp_in, fp_gid_map);

  save_db_globals();
}

void create_db_part(FILE* fp_in, FILE* fp_parts, char flat)
{
  char filename[256];
  strcat1(filename, PARAM::working_dir, PARAM::default_gidmap_filename);
  FILE* fp_gid_map = fopen(filename, "w+");

  if (fp_gid_map == NULL)
    error_exit("cannot create file '%s'", filename);

  gstore_init();

  graph1* org_graph = malloc_graph1_init("main_graph from create_db_part()");

  read_graph(org_graph, fp_in);

  if (flat == 0)
    read_parts_into_graph(org_graph, fp_parts);
  else if (flat == 1)
    generate_rnd_parts_into_graph(org_graph);
  else if (flat == 2)
    generate_parts_into_graph(org_graph);  

  int num_v = org_graph->num_v;
  int num_p = org_graph->num_p;
  int* part = org_graph->part;
  int* v_per_p = malloc1_set<int>(num_v, -1, "todo123");
  int* v_per_p_begin = malloc1<int>(num_p, "todo12");
  int* v_per_p_end = malloc1_set<int>(num_p, -1,"todo12");
  int tmp, i;

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

  printf("Finalizing...\n\n");

  if (PARAM::debug_mode) 
  {
    print_graph3(org_graph,v_per_p,v_per_p_begin,"before finalization");
    evaluate(org_graph, v_per_p, v_per_p_begin,"before finalization");
  }

  finalize(org_graph, v_per_p, v_per_p_begin);
  
  if (PARAM::debug_mode) 
  {
    print_graph3(org_graph,v_per_p,v_per_p_begin,"after finalization");
    //evaluate(org_graph, v_per_p, v_per_p_begin,"after finalization");
  }

  evaluate(org_graph, v_per_p, v_per_p_begin,"after finalization");

  free_all(2, v_per_p, v_per_p_begin);

  print_part_gid(org_graph, NULL, fp_gid_map);

  mem_create_disks_blks(org_graph->num_p, PARAM::max_disk_size);

  free_graph1(org_graph);

  read_to_blocks(fp_in, fp_gid_map);

  save_db_globals();
}
 
void gstore_init()
{
  GLOBALS::blk_writable = GLOBALS::blk_size - (4 + 3 + 2 + 2); //preliminary, todo: possibly now redundant due to change in find_blocksize
  FIRST_RUN = true;
  GLOBALS::dbl_avg_c_ratio = 0.0;
  GLOBALS::fp_org_size = NULL;
  calc_header(); 
  assign_rec_field_fkts();

  load_parameters();
  load_db_globals();
  QUERY::buffer_memory = PARAM::query_memory;
  QUERY::traverse_memory = QUERY::query_type == 0 ? 0 : PARAM::query_memory;
}

void close_down()
{
  print_ln("Goodbye...");
  print_ln("Press any key to exit");
  //_getch();
  exit(EXIT_SUCCESS);
};