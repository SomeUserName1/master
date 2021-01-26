#include "query_shared.h"

#include <time.h>

#include "structs.h"
#include "util_t.h"
#include "query.h"
#include "parameters.h"


/* Functions used for multiple query types */
//#include <g-store.h>

/*
 * Declared in query.h
 */
void evaluate_label(block1 p, header1 h, g_id first_gid, int v_rel, 
		std::unordered_map< unsigned int, Label > &label, g_id v_gid) {
	if (QUERY::pred_tree_start_with->evaluate(p, h, first_gid | v_rel, 
				1, 0, false)) {
		label[v_gid] = Source;
		//printf("set Source\n");
	} else if (QUERY::pred_tree_end_with->evaluate(p, h, first_gid | v_rel)) {
		label[v_gid] = Destination;
		//printf("set Destination\n");
	} else if (QUERY::pred_tree_through->evaluate(p, h, first_gid | v_rel)) {
		label[v_gid] = Through;
		//printf("set Through\n");
	} else {
		label[v_gid] = Unfit;
		//printf("set Unfit\n");
	}
}


void query_vertex()
{
  QUERY::output_buffer = malloc1<char>(MAXLINE + 1, "todo");

  query* q;
  switch (QUERY::query_type)
  {
  case q_vertex_:
    q = new q_vertex();
    break;
  
  case q_trav_:
    q = new q_trav();
    break;
  
  case q_path_:
    q = new q_path_set(); // select implementation here
    break;
  
  case q_spath_:
    q = new q_spath_set(); // select implementation here
    break;
  
  case q_spath_tree_:
    q = new q_spath_tree();
    break;

  case q_path_seq_:
    q = new q_path_seq();
    break;
  }


  LARGE_INTEGER start, finish, freq;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  q->start();

  QueryPerformanceCounter(&finish);
  printf("Execution time : %.3fs", (finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);

  if (QUERY::query_type == q_path_ || QUERY::query_type == q_spath_  || QUERY::query_type == q_path_seq_)
    printf("  (path was found after %.3fs)", (QUERY::xtra_start_print.QuadPart - start.QuadPart) / (double)freq.QuadPart);
  putchar('\n');

  if (!QUERY::must_close_output && (finish.QuadPart - start.QuadPart) / (double)freq.QuadPart > 60)
    printf ("   **Tip: Using SPOOL TO can speed up queries\n"); 

  if (PARAM::count_io)
    printf("Number of pages read: %d    Number of disk accesses: %d\n", QUERY::xtra_cnt_IO_single, QUERY::xtra_cnt_IO_cons);
  putchar('\n');
    
  if (PARAM::print_query_time)
  {
    FILE* fp = fopen(PARAM::query_time_filename, "a");
    if (fp == NULL)
      printf("Warning: could not write query time to file '%s'\n", PARAM::query_time_filename);
    else
    {
      time_t t = time(0);
      struct tm * now = localtime( & t );
      
      fprintf(fp,"%04d-%02d-%02d.%02d:%02d:%02d;%s;%d;%d;%.3f", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec,
                           QUERY::query_type == q_vertex_ ? "VERTEX" : 
                          (QUERY::query_type == q_trav_ ? "TRAV" : 
                          (QUERY::query_type == q_path_ ? "PATH" : 
                          (QUERY::query_type == q_spath_ ? "SPATH" : 
                          (QUERY::query_type == q_spath_tree_ ? "SPTREE" : 
                          (QUERY::query_type == q_path_seq_ ? "PSEQ" : "OTHER"))))),
                          PARAM::query_memory / 1024, 
                          GLOBALS::blk_size / 1024, 
                          (finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);
      
      if (QUERY::query_type == q_path_ || QUERY::query_type == q_spath_  || QUERY::query_type == q_path_seq_)
          fprintf(fp, ";%.3f;%d", (QUERY::xtra_start_print.QuadPart - start.QuadPart) / (double)freq.QuadPart, QUERY::xtra_path_len);

      if (PARAM::count_io)
          fprintf(fp, ";%d;%d", QUERY::xtra_cnt_IO_single, QUERY::xtra_cnt_IO_cons);

      fprintf(fp, "\t(time;query;memory_kb;page_kb;time_s");
      if (QUERY::query_type == q_path_ || QUERY::query_type == q_spath_  || QUERY::query_type == q_path_seq_)
        fprintf(fp, ";path_time_s;path_len");
      if (PARAM::count_io)
        fprintf(fp, ";io_page;io_access");

      fputc(')', fp);
      fputc('\n', fp);
    }
    fclose(fp);
  }

  if (QUERY::must_close_output)
    fclose(QUERY::output_fp);
  
  delete q;
  free(QUERY::output_buffer);
}

void print_query_header()
{
  char* str0 = QUERY::output_buffer, *str1;
  
  for (int i = 0; i < QUERY::select_list_len; i++)
  {
    str1 = QUERY::select_list[i].field_name;
    while (*str1 != '\0')
      *str0++ = *str1++;
    *str0++ = PARAM::output_delimiter;
  }

  *(str0 - 1) = '\n';
  
  fwrite(QUERY::output_buffer,str0 - QUERY::output_buffer,1,QUERY::output_fp);
}

template <typename T_>
void print_found_path(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<T_>* bm3, g_id* p_v_offset, g_id* v_found_paths)
{
  bm3->ref_add(to_bid, to_slt);
  uint4* tmp_uint4_ptr = v_found_paths + p_v_offset[to_bid] + to_slt;
  int cnt_path_len = 0;

  if (*tmp_uint4_ptr != PATH_FROM_GID)
    do
    {
      cnt_path_len++;
      tmp_uint4_ptr = v_found_paths + *tmp_uint4_ptr;
      to_bid = get_lower_idx_n(uint4(tmp_uint4_ptr - v_found_paths), p_v_offset, GLOBALS::num_parts);
      to_slt = tmp_uint4_ptr - (v_found_paths + p_v_offset[to_bid]);
      bm3->ref_add(to_bid, to_slt);
    } while (*tmp_uint4_ptr != PATH_FROM_GID);

  printf("Found %spath of length %d.\n", QUERY::query_type == q_spath_ ? "shortest " : "", cnt_path_len);
  QUERY::xtra_path_len = cnt_path_len;
  QueryPerformanceCounter(&QUERY::xtra_start_print); 

  if (!QUERY::must_close_output)
    printf("\n");

  print_query_header();

  print_found_path_common(bm1, bm3);
};

template void print_found_path<bv_list_slt>(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<bv_list_slt>* bm3, g_id* p_v_offset, g_id* v_found_paths);
template void print_found_path<bv_list_slt_start_with>(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<bv_list_slt_start_with>* bm3, g_id* p_v_offset, g_id* v_found_paths);

void print_found_path_seq(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<bv_list_slt_seq>* bm3, g_id* p_v_offset, g_id** v_found_paths, int seq)
{
  //the pointer tmp_uint4_ptr is used to extract the current level
  //the value behind the pointer is the offset to the next lower level

  bm3->ref_add(to_bid, to_slt); //this is seq and end with
  uint4* tmp_uint4_ptr = v_found_paths[seq] + p_v_offset[to_bid] + to_slt; //pointer: end with, value: end with - 1
  int cnt_path_len = 0;

  if (*tmp_uint4_ptr != PATH_FROM_GID)
    do
    {
      cnt_path_len++;
      seq = (seq + QUERY::thr_seq_cnt - 1) % QUERY::thr_seq_cnt;
      tmp_uint4_ptr = v_found_paths[seq] + *tmp_uint4_ptr; 
      to_bid = get_lower_idx_n(uint4(tmp_uint4_ptr - v_found_paths[seq]), p_v_offset, GLOBALS::num_parts);
      to_slt = tmp_uint4_ptr - (v_found_paths[seq] + p_v_offset[to_bid]);
      bm3->ref_add(to_bid, to_slt);
    } while (*tmp_uint4_ptr != PATH_FROM_GID);

    printf("Found path of length %d.\n", cnt_path_len);
    QUERY::xtra_path_len = cnt_path_len;
    QueryPerformanceCounter(&QUERY::xtra_start_print); 

    if (!QUERY::must_close_output)
      printf("\n");

    print_query_header();

    print_found_path_common(bm1, bm3);
}

template <typename T_>
void print_found_path_common(bufmgr1* bm1, bufmgr3<T_>* bm3)
{
  b_id to_bid;
  b_sc to_slt;
  uint4 rownum = 1;
  int lvl = 0;
  block1 b;
  header1 h;
  bool is_cycle = false;
  int sel_path_len = 0;
  g_id gid;

  while (bm3->ref_get(to_bid, to_slt))
  {
    if (rownum >= QUERY::max_rownum)
      break;

    if (lvl >= QUERY::last_lvl_q)
      break;

    bm1->get_block(b, to_bid);  //no disk access if block is in buffer

    h = blk_get_header(b, to_slt);

    gid = get_first_gid(to_bid) | to_slt;

    if (lvl == 0 && QUERY::cnt_sel_root > 0)
      add_to_sel_root_arr(b, h);

    if (QUERY::sel_path != NULL)
      add_to_sel_path(b, h, sel_path_len);

    if (QUERY::sel_iscycle)
      is_cycle = bm3->identify_path_cycle(b + get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off, b + get_header(h + GLOBALS::ee_in_h), b + get_header(h + GLOBALS::ee_end_in_h), lvl);

    if (QUERY::pred_tree_where->evaluate(b, h, gid, rownum, lvl, is_cycle))
      print_query_rec_new(b, h, rownum++, gid, lvl, sel_path_len, is_cycle);

    lvl++;
  }

  if (!QUERY::must_close_output)
    printf("\n");

  printf("Found %u records.\n\n", rownum-1);
}

template void print_found_path_common<bv_list_slt>(bufmgr1* bm1, bufmgr3<bv_list_slt>* bm3);
template void print_found_path_common<bv_list_slt_seq>(bufmgr1* bm1, bufmgr3<bv_list_slt_seq>* bm3);
template void print_found_path_common<bv_list_slt_start_with>(bufmgr1* bm1, bufmgr3<bv_list_slt_start_with>* bm3);


void print_query_rec_new(block1 b, header1 h, uint4 rownum, g_id gid, int lvl, int &sel_path_len, bool iscycle)
{
  int j;
  char* str0 = QUERY::output_buffer;
  char* str1;
  int tmp;
    
  for (int i = 0; i < QUERY::select_list_len; i++)
  {
    switch (QUERY::select_list[i].field_datatype)
    {
    case (sel_gid_):
      inttostr_adv(str0, gid);
      break;

    case(sel_count_edges_):
      inttostr_adv(str0, (get_header(h + GLOBALS::ee_in_h) - get_header(h + GLOBALS::ie_in_h) - GLOBALS::ie_fix_off) / GLOBALS::ie_size + 
        (get_header(h + GLOBALS::ee_end_in_h) - get_header(h + GLOBALS::ee_in_h) ) / 4);
      break;
    
    case(sel_isleaf_):
      if (get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off == get_header(h + GLOBALS::ee_in_h)
        && get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off == get_header(h + GLOBALS::ee_end_in_h))
        *str0++='1';
      else
        *str0++='0';
      break;

    case(sel_rownum_):
      inttostr_adv(str0, rownum);
      break;

    case(sel_lvl_):
      inttostr_adv(str0, lvl);
      break;

    case(sel_iscycle_):
      if (iscycle)
        *str0++ = '1';
      else 
        *str0++ = '0';
      break;

    case(sel_lpad_):
      for (j = 0; j < lvl; j++)
        *str0++ = QUERY::select_list[i].sel_char;
      (QUERY::select_list[i].*(QUERY::select_list[i].eff_append_to_str))(str0, b, h);
      break;

    case(sel_path_):
      str1 = QUERY::sel_path;
      tmp = sel_path_len;
      while (tmp-- > 0)
        *str0++ = *str1++;
      break;

    case(sel_root_):
      str1 = QUERY::sel_root_arr[QUERY::select_list[i].sel_char];
      while (*str1 != '\0') 
        *str0++ = *str1++;
      break;

    default:
      (QUERY::select_list[i].*(QUERY::select_list[i].eff_append_to_str))(str0,b,h);
      break;
    }
    
    *str0++ = PARAM::output_delimiter;
  }

  *(str0 - 1) = '\n';
  
  fwrite(QUERY::output_buffer,str0 - QUERY::output_buffer,1,QUERY::output_fp);
}

void push_on_output(char* str1)
{
  fputs(str1, QUERY::output_fp);
}

void add_to_sel_path(block1 b, header1 h, int &sel_path_len)
{
  char* str1 = QUERY::sel_path + sel_path_len;
  *str1++ = QUERY::sel_path_ptr->sel_char;
  (QUERY::sel_path_ptr->*(QUERY::sel_path_ptr->eff_append_to_str))(str1, b, h);
  sel_path_len = str1 - QUERY::sel_path;
}

void add_to_sel_root_arr(block1 b, header1 h)
{
  char* str1;

  for (int i = 0; i < QUERY::select_list_len; i++)
    if (QUERY::select_list[i].field_datatype == sel_root_)
    {
      str1 = QUERY::sel_root_arr[QUERY::select_list[i].sel_char];
      (QUERY::select_list[i].*(QUERY::select_list[i].eff_append_to_str))(str1, b, h);
      *str1 = '\0';
    }  
}

//TODO: check for if "if if - else "match -- HOW?? :/

void register_finished()
{
	QueryPerformanceCounter(&QUERY::xtra_start_print);
}