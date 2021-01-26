#include "evaluate.h"

#include "util.h"
#include "util_t.h"
#include "parameters.h"

/* called at the end of G-Store's stroage algorithm 
to print statistics into file stats.g */
//#include <g-store.h>

void evaluate(graph1* graph, int* v_per_p, int* v_per_p_begin, char* text /*= ""*/)
{
  int num_v      = graph->num_v;
  int* offset_e  = graph->offset_e;
  int* v_w       = graph->v_w;
  int* e_to      = graph->e_to;
  int* e_w       = graph->e_w;
  int* map_in_c  = graph->map_in_c;
  int* part      = graph->part;
  int* p_w       = graph->p_w;
  int  num_p     = graph->num_p;
  int  c_level   = graph->c_level;

  static char* mode = "w";
 
  int i, j, k;
  int cnt_ext, cnt_int;
  int8 cnt_tens;
  p_stats p_st;
  g_stats g_st;
  g_st.init();

  char filename1[256];
  //char filename2[256];
  
  strcat1(filename1, PARAM::working_dir, "stats.g");
  //strcat1(filename2, PARAM::working_dir, "stats_sum.g");
  
  FILE* fp1 = fopen(filename1, mode);
  //FILE* fp2 = fopen(filename2, mode);
  FILE* fp;

  if (fp1 == NULL)
    error_exit("cannot open file %s to write graph stats", filename1);
  /*if (fp2 == NULL)
    error_exit("cannot open file %s to write graph stats", filename2);*/

  if (graph == NULL) 
  {
    fprintf(fp1, "  (EMPTY GRAPH)\n\n");
//    fprintf(fp2, "  (EMPTY GRAPH)\n\n");
    fclose(fp1);
//   fclose(fp2);
    return;
  }

  fp = fp1;

  //for (fp = fp1, i = 0 ; i < 2 ; fp = fp2, i++)
  //{
    fprintf(fp, "***STATS (c_level = %d, threshold = %lld, avg_c_ratio = %f, block_size = %d)  %s\n", 
      graph->c_level, 
      get_threshold(graph->c_level), 
      GLOBALS::dbl_avg_c_ratio,
      GLOBALS::blk_size,
      text);
  
    fprintf(fp, "            %10s %10s %10s %10s %10s %20s  ##  %12s %12s %12s  ##  "
      "%10s %10s (%5s) / %10s %10s (%5s)  ##  %10s %10s (%5s) / %10s %10s (%5s)  ##  "
      "%20s %10s (%5s) / %20s %10s (%5s)\n",
                "part",
                "part w",
                "num_v",
                "int ew",
                "ext ew",
                "tension",
                "avg int ew",
                "avg ext ew",
                "avg tensio",
                "max int ew", "id", "%",
                "min int ew", "id", "%",
                "max ext ew", "id", "%",
                "min ext ew", "id", "%",
                "max tension", "id", "%",
                "min tension", "id", "%"
            );
  //}

  for (i = 0; i < num_p; i++)
  {
    j = v_per_p_begin[i];
    
    assert (j != -1);

    p_st.reset();

    do 
    {
      cnt_ext = cnt_int = 0;
      cnt_tens = 0L;
      
      for (k = offset_e[j]; k < offset_e[j+1]; k++)
      {
        if (part[e_to[k]] == i)
          cnt_int += e_w[k];
        else
        {
          cnt_ext += e_w[k];
          cnt_tens += e_w[k] * abs1(part[e_to[k]] - i);
        }
      }
      
      p_st.update(j, cnt_int, cnt_ext, cnt_tens);

    } while ((j = v_per_p[j]) != -1);
    
    fprintf(fp1,
      "  PART      %10d %10d %10d %10d %10d %20lld  ##  %12.1f %12.1f %12.1f  ##  "
      "%10d %10d (%5.3f) / %10d %10d (%5.3f)  ##  %10d %10d (%5.3f) / %10d %10d (%5.3f)  ##  "
      "%20lld %10d (%5.3f) / %20lld %10d (%5.3f)\n",
      i,
      p_w[i],
      p_st.cnt_num_v_p, 
      p_st.cnt_int_p, 
      p_st.cnt_ext_p,
      p_st.cnt_tens_p,
      p_st.cnt_int_p / double(p_st.cnt_num_v_p),
      p_st.cnt_ext_p / double(p_st.cnt_num_v_p),
      double(p_st.cnt_tens_p / double(p_st.cnt_num_v_p)),

      p_st.max_int_v.sc, p_st.max_int_v.idx, (p_st.cnt_int_p == 0.0 ? 0.0 : p_st.max_int_v.sc / double(p_st.cnt_int_p)),
      p_st.min_int_v.sc, p_st.min_int_v.idx, (p_st.cnt_int_p == 0.0 ? 0.0 : p_st.min_int_v.sc / double(p_st.cnt_int_p)),
      p_st.max_ext_v.sc, p_st.max_ext_v.idx, (p_st.cnt_ext_p == 0.0 ? 0.0 : p_st.max_ext_v.sc / double(p_st.cnt_ext_p)),
      p_st.min_ext_v.sc, p_st.min_ext_v.idx, (p_st.cnt_ext_p == 0.0 ? 0.0 : p_st.min_ext_v.sc / double(p_st.cnt_ext_p)),
      p_st.max_tens_v.sc, p_st.max_tens_v.idx, (p_st.cnt_tens_p == 0.0 ? 0.0 : double(p_st.max_tens_v.sc / double(p_st.cnt_tens_p))),
      p_st.min_tens_v.sc, p_st.min_tens_v.idx, (p_st.cnt_tens_p == 0.0 ? 0.0 : double(p_st.min_tens_v.sc / double(p_st.cnt_tens_p)))
      );

    g_st.update(i, p_st, p_w[i]);
  }

  char tot1[] = "\n"
                "TOTAL   (p) %10d %10s %10s %10lld %10lld %20lld  ##  %12.1f %12.1f %12.1f  ##  "
                "%10d %10d (%5.3f) / %10d %10d (%5.3f)  ##  %10d %10d (%5.3f) / %10d %10d (%5.3f)  ##  "
                "%20lld %10d (%5.3f) / %20lld %10d (%5.3f)\n";
  
  char tot2[] = "TOTAL   (v) %10s %10s %10d  %10s %10s %20s      %12s %12s %12s     "
                "%10d %10d  %5s  / %10d %10d  %5s   ##  %10d %10d  %5s  / %10d %10d  %5s   ##  "
                "%20lld %10d  %5s  / %20lld %10d\n";

  char tot3[] = "TOTAL (p_w) %10s %10lld %10s %10s %10s %20s      %12.1f %12s %12s      "
                "%10d %10d  %5s  / %10d %10d\n\n";

  fp = fp1;
  //for (fp = fp1, i = 0 ; i < 2 ; fp = fp2, i++)
  //{
    fprintf(fp, tot1,         
                g_st.cnt_num_p_g,
                "", "",
                g_st.cnt_int_g, 
                g_st.cnt_ext_g,
                g_st.cnt_tens_g,
                double(g_st.cnt_int_g / double(g_st.cnt_num_p_g)),
                double(g_st.cnt_ext_g / double(g_st.cnt_num_p_g)),
                double(g_st.cnt_tens_g / double(g_st.cnt_num_p_g)),
              
                g_st.max_int_p.sc, 
                g_st.max_int_p.idx,
                (g_st.cnt_ext_g == 0.0 ? 0.0 : g_st.max_int_p.sc / double(g_st.cnt_int_g)),
                g_st.min_int_p.sc, 
                g_st.min_int_p.idx,
                (g_st.cnt_ext_g == 0.0 ? 0.0 : g_st.min_int_p.sc / double(g_st.cnt_int_g)),
                g_st.max_ext_p.sc, 
                g_st.max_ext_p.idx,
                (g_st.cnt_ext_g == 0.0 ? 0.0 : g_st.max_ext_p.sc / double(g_st.cnt_ext_g)),
                g_st.min_ext_p.sc, 
                g_st.min_ext_p.idx,
                (g_st.cnt_ext_g == 0.0 ? 0.0 : g_st.min_ext_p.sc / double(g_st.cnt_ext_g)),
              
                g_st.max_tens_p.sc, 
                g_st.max_tens_p.idx,
                (g_st.cnt_ext_g == 0.0 ? 0.0 : double(g_st.max_tens_p.sc / double(g_st.cnt_tens_g))),
                g_st.min_tens_p.sc, 
                g_st.min_tens_p.idx,
                (g_st.cnt_ext_g == 0.0 ? 0.0 : double(g_st.min_tens_p.sc / double(g_st.cnt_tens_g)))
           );
    
    fprintf(fp, tot2,
                "", "",
                g_st.cnt_num_v_g,
                "", "", "", "", "", "",

                g_st.max_int_v.sc, 
                g_st.max_int_v.idx, 
                "",
                g_st.min_int_v.sc,
                g_st.min_int_v.idx, 
                "",
                g_st.max_ext_v.sc, 
                g_st.max_ext_v.idx, 
                "",
                g_st.min_ext_v.sc,
                g_st.min_ext_v.idx,
                "",
              
                g_st.max_tens_v.sc,
                g_st.max_tens_v.idx,
                "",
                g_st.min_tens_v.sc, 
                g_st.min_tens_v.idx
           );
    
    fprintf(fp, tot3,
                "",
                g_st.cnt_p_w,
                "", "", "", "",
                double(g_st.cnt_p_w / double(g_st.cnt_num_p_g)),
                "", "",
              
                g_st.max_w_p.sc,
                g_st.max_w_p.idx,
                "",
                g_st.min_w_p.sc,
                g_st.min_w_p.idx
           );
  //}
  fclose(fp1);
  //fclose(fp2);
  mode = "a";
}