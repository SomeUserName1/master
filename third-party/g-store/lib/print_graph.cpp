#include "print_graph.h"

#include "structs.h"
#include "util.h"
#include "util_t.h"
#include "parameters.h"

/* print_graph1-3() are ised during debugging to printa graph to text file. 
Different levels of detail.
print_part_gid() to print _gidmap.g  and _parts.g. See dissertation Section 6.7 */
//#include <g-store.h>

void print_graph1(graph1 *graph, char* text, bool print_coarser)
{
  static char* mode = "w";
  char filename[256];
  sprintf(filename,"%sgraph_debug1.g",PARAM::working_dir);

  FILE *fp_outp = fopen(filename, mode);
  if (fp_outp == NULL)
    error_exit("cannot open graph file %s", filename);

  int i, max_v, max_e;

  fprintf(fp_outp, "**************%s\n", text);

  if (graph==NULL) {
    fprintf(fp_outp, "%-14s\n__________\n\n", "EMPTY GRAPH");
    fclose(fp_outp);
    return;
  }
  int sum = 0;

  do 
  {
    max_v = min1(3000, graph->num_v);
    max_e = min1(12000, graph->num_e);

    fprintf(fp_outp, "%-14s", "(index)");
    for (i=0; i < max_e; i++) 
      fprintf(fp_outp,"%5d ",i);
    fprintf(fp_outp, "\n");

    fprintf(fp_outp, "%-14s", "num_v");
    fprintf(fp_outp, "%5d\n", graph->num_v);

    fprintf(fp_outp, "%-14s", "num_e");
    fprintf(fp_outp, "%5d\n", graph->num_e);

    fprintf(fp_outp, "%-14s", "offset_e");
    if (graph->offset_e!=NULL){
      for (i=0; i < max_v + 1; i++)
        fprintf(fp_outp,"%5d ",graph->offset_e[i]);
    } else fprintf(fp_outp,"%s","NULL");
    fprintf(fp_outp, "\n");

    fprintf(fp_outp, "%-14s", "v_w");
    if (graph->v_w!=NULL){
      for (i=0; i < max_v; i++) {
        fprintf(fp_outp,"%5d ",graph->v_w[i]);
        sum+=graph->v_w[i];
      }
      fprintf(fp_outp, "\n%-14s(%5d)", "",sum);
      sum = 0;
    } else fprintf(fp_outp,"%s","NULL");
    fprintf(fp_outp, "\n");

    fprintf(fp_outp, "%-14s", "e_out_to");
    if (graph->e_to!=NULL){
      for (i=0; i < max_e; i++) 
        fprintf(fp_outp,"%5d ",graph->e_to[i]);
    } else fprintf(fp_outp,"%s","NULL");
    fprintf(fp_outp, "\n");

    fprintf(fp_outp, "%-14s", "e_out_w");
    if (graph->e_w!=NULL){
      for (i=0; i < max_e; i++){
        fprintf(fp_outp,"%5d ",graph->e_w[i]);
        sum+=graph->e_w[i];
      }
      fprintf(fp_outp, "\n%-14s(%5d)", "",sum);
      sum = 0;
    } else fprintf(fp_outp,"%s","NULL");
    fprintf(fp_outp, "\n");

    fprintf(fp_outp, "%-14s", "map_in_c");
    if (graph->map_in_c!=NULL){
      for (i=0; i < max_v; i++) 
        fprintf(fp_outp,"%5d ",graph->map_in_c[i]);
    } else fprintf(fp_outp,"%s","NULL");
    fprintf(fp_outp, "\n");

    fprintf(fp_outp,"__________\n\n");

  } while (print_coarser && (graph=graph->coarser) !=NULL);

  fclose(fp_outp);
  mode = "a";
}

void print_graph2(graph1 *graph, char* text, bool print_coarser)
{
  static char* mode = "w";
  char filename[256];
  sprintf(filename,"%sgraph_debug2.g",PARAM::working_dir);

  FILE *fp_outp = fopen(filename, mode);
  if (fp_outp == NULL)
    error_exit("cannot open graph file %s", filename);



  fprintf(fp_outp, "**************%s            c_level=%d (v_w)[map_in_c]{part}\n", text, graph->c_level);

  if (graph==NULL) {
    fprintf(fp_outp, "%-14s\n__________\n\n", "EMPTY GRAPH");
    fclose(fp_outp);
    return;
  }

  int i, j = 0, sum = 0;

  do {
    for (i=0; i < graph->num_v; i++) {
      fprintf(fp_outp, "%4d ", i);
      fprintf(fp_outp, "(%4d) ", graph->v_w[i]);
      if (graph->map_in_c!=NULL)
        fprintf(fp_outp, "[%4d] ", graph->map_in_c[i]);
      if (graph->part!=NULL)
        fprintf(fp_outp, "{%4d} ", graph->part[i]);
      while (j < graph->offset_e[i+1])
      {
        if (graph->part==NULL)
          fprintf(fp_outp, "%d (%d) ", graph->e_to[j], graph->e_w[j]);
        else
          fprintf(fp_outp, "%d (%d; %d) ", graph->e_to[j], graph->e_w[j], graph->part[graph->e_to[j]]);
        j++;
      }
      fprintf(fp_outp, "\n");
    }
  } while (print_coarser && (graph=graph->coarser) !=NULL);

  fclose(fp_outp);
  mode = "a";
}


void print_graph3(graph1 *graph, int* v_per_p, int* v_per_p_begin, char* text)
{
  static char* mode = "w";
  char filename[256];
  sprintf(filename,"%sgraph_debug3.g",PARAM::working_dir);

  FILE *fp_outp = fopen(filename, mode);
  if (fp_outp == NULL)
    error_exit("cannot open graph file %s", filename);

  fprintf(fp_outp, "**************%s            c_level=%d (v_w)[map_in_c]{part}\n", text, graph->c_level);

  if (graph==NULL) {
    fprintf(fp_outp, "%-14s\n__________\n\n", "EMPTY GRAPH");
    fclose(fp_outp);
    return;
  }

  int i, j = 0, sum = 0;

  for (i = 0; i < graph->num_p; i++) 
  {
    fprintf(fp_outp, "PART %d:\n", i);
    j = v_per_p_begin[i];

    if (j == -1)
      continue;

    do
    {    
      fprintf(fp_outp, "%4d (%4d) ", j, graph->v_w[j]);
      if (graph->map_in_c!=NULL)
        fprintf(fp_outp, "[%4d] ", graph->map_in_c[j]);
      if (graph->part!=NULL)
        fprintf(fp_outp, "{%4d} ", graph->part[j]);
      for (int k = graph->offset_e[j]; k < graph->offset_e[j+1]; k++)
      {
        if (graph->part==NULL)
          fprintf(fp_outp, "%d (%d) ", graph->e_to[k], graph->e_w[k]);
        else
          fprintf(fp_outp, "%d (%d; %d) ", graph->e_to[k], graph->e_w[k], graph->part[graph->e_to[k]]);
      }
      fprintf(fp_outp, "\n");
    } while ((j = v_per_p[j]) != -1);
  }

  fclose(fp_outp);
  mode = "a";
}


void print_part_gid(graph1 *graph, FILE* part_file, FILE* gid_map_file)
{
  int* part = graph->part;
  int num_p = graph->num_p;
  int num_v = graph->num_v;
  int i, tmp;

  GLOBALS::num_parts = num_p;
  
  if (GLOBALS::ie_size == 1)
    GLOBALS::v_p_blk = malloc1<char>(num_p,"todo");
  else
    GLOBALS::v_p_blk = malloc1<char>(2 * num_p,"todo");

  char* v_p_blk_ptr = (char*) GLOBALS::v_p_blk;

  int* nxt_gid = malloc1<int>(graph->num_p,"todo");

  for (i = 0; i < num_p; i++)
  {
    nxt_gid[i] = GLOBALS::max_rec_p_page * i;
    set_ie_adv(v_p_blk_ptr, 0);
  }

  for (i = 0; i < num_v; i++)
  {
    tmp = part[i];
    if (part_file != NULL)
      fprintf(part_file, "%d\n", tmp);
    fprintf(gid_map_file, "%d\n", nxt_gid[tmp]);
    nxt_gid[tmp]++;
    v_p_blk_ptr = GLOBALS::v_p_blk + (GLOBALS::ie_size * tmp);
    set_ie(v_p_blk_ptr, get_ie(v_p_blk_ptr) + 1);
  }

  if (part_file != NULL)
    fclose(part_file);
}
