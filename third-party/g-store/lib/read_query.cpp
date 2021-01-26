#include "read_query.h"

#include "structs.h"
#include "util_t.h"
#include "util.h"
#include "pred_tree.h"
#include "query/query_shared.h"
#include "parameters.h"

/* Parser for G-Store's query engine. See dissertation Figures 11 and 12  */
//#include <g-store.h>

// todo: * and nothing else: no *, field,...

namespace QUERY 
{
  uint2 traverse_lvl;
  pred_tree_elem* pred_tree_where;
  pred_tree_elem* pred_tree_through;

  //new:
  std::vector<pred_tree_elem*> pred_tree_through_seq;
  int thr_seq_cnt;

  pred_tree_elem* pred_tree_start_with;
  pred_tree_elem* pred_tree_end_with;
  rec_struct* select_list;
  int select_list_len;
  uint4 buffer_memory;
  FILE* output_fp;
  bool must_close_output;

  char* sel_path;
  rec_struct* sel_path_ptr;

  char cnt_sel_root;
  char** sel_root_arr;
  bool sel_iscycle;

  bool nocycle;

  uint4 max_rownum;
  int last_lvl_q;
  int last_lvl_p;

  uint4 traverse_memory;
  
  query_types query_type;
  char* output_buffer;

  int xtra_path_len;
  LARGE_INTEGER xtra_start_print;
  int xtra_cnt_IO_single;
  int xtra_cnt_IO_cons;
}

void read_query(char* line, bool first_q /*=true*/)
{
  if (*GLOBALS::graph_name == '\0')
    error_exit("A query can only be executed after a graph has been loaded.");

  char* str0 = line;
  char* str1;
  char tmp_str[64];
  char filename[256];
  bool first_fld = true;
  int i;
  int str_len = 0;
  pred_parser p_parser;
  int tmp = -1;
  bool used_extended = false;

  std::vector<rec_struct> select_list;

  str_eat_ws(str0);

  str_tok_cmp_adv_qd(str0, "SELECT", 4);
    
  QUERY::cnt_sel_root = 0;
  QUERY::sel_path = NULL;
  QUERY::sel_iscycle = false;
  QUERY::last_lvl_q = INT_MAX;
  QUERY::last_lvl_p = PARAM::max_traverse_depth - 1;
  QUERY::max_rownum = UINT_MAX;
  QUERY::xtra_cnt_IO_cons = QUERY::xtra_cnt_IO_single = 0;

  str_eat_ws(str0);

  if (*str0 == '*')
  {
    str0++;
    for (i = 0; i < GLOBALS::num_fields; i++)
    {
      if (GLOBALS::rec_fields[i].field_datatype != skip_)
        select_list.push_back(GLOBALS::rec_fields[i]);
    }
  }
  else
    for(;;)
    {  
      str_eat_ws(str0);

      // PSEUDOCOLUMN A
      if (str_tok_cmp_adv_qd(str0, "GID", 1))
      {
        select_list.push_back(rec_struct(sel_gid_, "GID"));
      }
      else if (str_tok_cmp_adv_qd(str0, "COUNT_EDGES", 1))
      {
        select_list.push_back(rec_struct(sel_count_edges_, "COUNT_EDGES"));
      }
      else if (str_tok_cmp_adv_qd(str0, "ISLEAF", 1))
      {
        select_list.push_back(rec_struct(sel_isleaf_, "ISLEAF"));
      }
      
      // PSEUDOCOLUMN B
      else if (str_tok_cmp_adv_qd(str0, "ROWNUM", 1))
      {
        select_list.push_back(rec_struct(sel_rownum_, "ROWNUM"));
      }
      else if (str_tok_cmp_adv_qd(str0, "LEVEL", 1))
      {
        used_extended = true;
        select_list.push_back(rec_struct(sel_lvl_, "LEVEL"));
      }
      else if (str_tok_cmp_adv_qd(str0, "ISCYCLE", 1))
      {
        used_extended = true;
        QUERY::sel_iscycle = true;
        select_list.push_back(rec_struct(sel_iscycle_, "ISCYCLE"));
      }

      // PSEUDOCOLUMN C
      else if (str_tok_cmp_adv_qd(str0, "PASTE_EDGES", 1))
      {
        used_extended = true;
        str_eat_ws_tok_qd(str0, '(');
        str_eat_ws(str0);
        select_list.push_back(GLOBALS::rec_fields[parse_field_id_adv(str0)]);
        select_list.back().field_datatype = sel_paste_edges_;
        str_eat_ws_tok_qd(str0, ')');

        str1 = tmp_str;
        str_app_adv(str1, "PASTE_EDGES(");
        str_app_adv(str1, select_list.back().field_name);
        str_app_adv(str1, ')');
        str_app_adv(str1, '\0'); 
        strcpy(select_list.back().field_name, tmp_str);
      }
      else if (str_tok_cmp_adv_qd(str0, "LPAD", 1))
      {
        used_extended = true;
        str_eat_ws_tok_qd(str0, '(');
        str_eat_ws_tok_qd(str0, '\'');
        str1 = str0++;
        str_eat_ws_tok_qd(str0, '\'');
        str_eat_ws_tok_qd(str0, ',');
        str_eat_ws(str0);
        if (!str_tok_cmp_adv_qd(str0, "LEVEL", 1))
          error_exit("Expected 'LEVEL' in query definition.\nNot parsed: %s", str0);
        str_eat_ws_tok_qd(str0, ')');
        str_eat_ws(str0);
        if (!str_tok_cmp_adv_qd(str0, "||", 1))
          error_exit("Expected '||' in query definition.\nNot parsed: %s", str0);
        str_eat_ws(str0);
        select_list.push_back(GLOBALS::rec_fields[parse_field_id_adv(str0)]);
        select_list.back().sel_char = *str1;
        select_list.back().field_datatype = sel_lpad_;
        
        str1 = tmp_str;
        str_app_adv(str1, "LPAD(\'");
        str_app_adv(str1, select_list.back().sel_char);
        str_app_adv(str1, "\',LEVEL)||");
        str_app_adv(str1, select_list.back().field_name);
        str_app_adv(str1, '\0');
        strcpy(select_list.back().field_name, tmp_str);
      }
      else if (str_tok_cmp_adv_qd(str0, "PATH", 1))
      {
        used_extended = true;
        if (QUERY::sel_path != NULL)
          error_exit("Only one PATH may be selected in a query.");
        
        tmp = select_list.size(); //used below
        str_eat_ws_tok_qd(str0, '(');
        select_list.push_back(GLOBALS::rec_fields[parse_field_id_adv(str0)]);
        str_eat_ws_tok_qd(str0, ',');
        str_eat_ws_tok_qd(str0, '\'');
        select_list.back().sel_char = *str0++;
        str_eat_ws_tok_qd(str0, '\'');
        str_eat_ws_tok_qd(str0, ')');
        select_list.back().field_datatype = sel_path_;
        QUERY::sel_path = malloc1<char>(MAXLINE + 1,"todo");

        str1 = tmp_str;
        str_app_adv(str1, "PATH(");
        str_app_adv(str1, select_list.back().field_name);
        str_app_adv(str1, ",\'");
        str_app_adv(str1, select_list.back().sel_char);
        str_app_adv(str1, "\')");
        str_app_adv(str1, '\0');
        strcpy(select_list.back().field_name, tmp_str);
      }
      else if (str_tok_cmp_adv_qd(str0, "SOURCE", 1))
      {
        used_extended = true;
        if (QUERY::cnt_sel_root == 127)
          error_exit("Only 127 ROOT may be selected in a query.");

        str_eat_ws_tok_qd(str0, '(');
        str_eat_ws(str0);
        select_list.push_back(GLOBALS::rec_fields[parse_field_id_adv(str0)]);
        select_list.back().field_datatype = sel_root_;
        select_list.back().sel_char = QUERY::cnt_sel_root++;
        str_eat_ws_tok_qd(str0, ')');

        str1 = tmp_str;
        str_app_adv(str1, "SOURCE(");
        str_app_adv(str1, select_list.back().field_name);
        str_app_adv(str1, ')');
        str_app_adv(str1, '\0'); 
        strcpy(select_list.back().field_name, tmp_str);
      }
      else
      {
        select_list.push_back(GLOBALS::rec_fields[parse_field_id_adv(str0)]);
      }
    
      str_eat_ws(str0);

      // get custom title, if any
      if (str_tok_cmp_qd(str0, "AS", 3))
      {
        str_eat_ws(str0);
        
        str_len = 0;
        str1 = tmp_str;
        while (!str_is_ws_qd(str0))
        {
          str_app_adv(str1, *str0++);
          if (++str_len == 31)
          {
            *str1 = '\0';
            error_exit("title for field name to long in query definition: %s..", str1);
          }
        }

        *str1 = '\0';
        strcpy(select_list.back().field_name, str1);
        str_eat_ws(str0);
      }

      if (str_tok_cmp_qd(str0, "WHERE", 2) 
        || str_tok_cmp_qd(str0, "TRAVERSE", 2) 
        || str_tok_cmp_qd(str0, "IN", 2) 
        || str_tok_cmp_qd(str0, "START WITH", 2) 
        || str_tok_cmp_qd(str0, "SPOOL", 2) 
        || str_tok_cmp_qd(str0, ";", 2))
        break;

      str_eat_ws_tok_qd(str0, ',');
    } // SELECT loop

  // Prepare data structures, loose the vectors
  QUERY::select_list_len = select_list.size();
  if (QUERY::select_list_len == 0)
    print_ln("Warning: Nothing selected");

  QUERY::select_list = (rec_struct*) malloc_b(QUERY::select_list_len * sizeof(rec_struct), "todo");
  memcpy(QUERY::select_list, &select_list[0], QUERY::select_list_len * sizeof(rec_struct));
  select_list.clear();
  
  QUERY::sel_root_arr = malloc1<char*>(QUERY::cnt_sel_root, "todo");
  if (QUERY::cnt_sel_root > 0)
  {
    QUERY::sel_root_arr[0] = malloc1<char>(QUERY::cnt_sel_root * GLOBALS::blk_writable, "todo");
    for (i = 1; i < QUERY::cnt_sel_root; i++)
      QUERY::sel_root_arr[i] = QUERY::sel_root_arr[0] + GLOBALS::blk_writable * i;
  }

  if (tmp != -1)
  {
    QUERY::sel_path_ptr = QUERY::select_list + tmp;
    QUERY::sel_path = malloc1<char>(MAXLINE + 1,"todo");
  }
  
  
  
  //////////////////////////////////////////////////////////////////////////
  str_eat_ws(str0);

  if (str_tok_cmp_adv_qd(str0, "IN", 3))
  {
    str_eat_ws(str0);
    if (str_tok_cmp_adv_qd(str0, "PATH", 3))
      QUERY::query_type = q_path_;
    else if (str_tok_cmp_adv_qd(str0, "SHORTEST PATH TREE", 3))
      QUERY::query_type = q_spath_tree_;
    else if (str_tok_cmp_adv_qd(str0, "SHORTEST PATH", 3))
      QUERY::query_type = q_spath_;
    else
      error_exit("Expecting token PATH, SHORT PATH, SHORTEST PATH, or SHORTEST PATH TREE "
        "in query definition.\nNot parsed: %s", str0);

    str_eat_ws(str0);
  }
  else
  {
    QUERY::query_type = q_vertex_;
    if (str_tok_cmp_adv_qd(str0, "WHERE", 3))
    {
      str_eat_ws(str0);
      QUERY::pred_tree_where = p_parser.parse(str0, false, used_extended, true);
      str0 += p_parser.get_str_pos() - str0;
      str_eat_ws(str0);
    }
    else
      QUERY::pred_tree_where = new pred_tree_elem_true;
  }
  
  //////////////////////////////////////////////////////////////////////////
  // ws ok

  if (str_tok_cmp_adv_qd(str0, "START WITH", 3))
  {
    if (QUERY::query_type == q_vertex_)
      QUERY::query_type = q_trav_;

    str_eat_ws(str0);
    
    if (QUERY::query_type == q_trav_ && str_tok_cmp_adv_qd(str0, "ANY", 1))
    {
      QUERY::pred_tree_start_with = new pred_tree_elem_true;
    } 
    else
    {
      QUERY::pred_tree_start_with = p_parser.parse(str0, false, used_extended, false);
      str0 += p_parser.get_str_pos() - str0;
    }
    
    str_eat_ws(str0);

    if (QUERY::query_type == q_path_ || QUERY::query_type == q_spath_)
    {
      str_tok_cmp_adv_qd(str0, "END WITH", 4);
      str_eat_ws(str0);
      QUERY::pred_tree_end_with = p_parser.parse(str0, true, used_extended, false);
      str0 += p_parser.get_str_pos() - str0;
      str_eat_ws(str0);
    }

    if (QUERY::query_type == q_trav_)
    {
      if (str_tok_cmp_adv_qd(str0, "NOCYCLE", 1))
        QUERY::nocycle = true;
      else
        QUERY::nocycle = false;
      str_eat_ws(str0);
    }
    else
    {
      if (str_tok_cmp_adv_qd(str0, "THROUGH", 3))
      {      
        str_eat_ws(str0);
        if (str_tok_cmp_adv_qd(str0, "SEQUENCE", 3))
        {
          if (QUERY::query_type != q_path_)
            error_exit("THROUGH SEQUENCE is currently supported in IN PATH queries");
          p_parser.parse_seq(str0);
          str0 += p_parser.get_str_pos() - str0;
          QUERY::query_type = q_path_seq_;
        } 
        else   
        {
          QUERY::pred_tree_through = p_parser.parse(str0, true, used_extended, false);
          str0 += p_parser.get_str_pos() - str0;
        }
        str_eat_ws(str0);
      }
      else
        QUERY::pred_tree_through = new pred_tree_elem_true;

      if (str_tok_cmp_adv_qd(str0, "WHERE", 3))
      {
        str_eat_ws(str0);
        QUERY::pred_tree_where = p_parser.parse(str0, false, used_extended, true);
        str0 += p_parser.get_str_pos() - str0;
        str_eat_ws(str0);
      }
      else
        QUERY::pred_tree_where = new pred_tree_elem_true;
    } 
  }
  else if (str_tok_cmp_qd(str0, "SPOOL", 2) || (*str0 == ';'))
  {
    // simple vertex query
    if (used_extended)
      error_exit("LEVEL, ISCYCLE, PASTE_EDGES, LPAD, PATH, and ROOT are not allowed in simple vertex queries.\n"
        "Not parsed: %s", str0);
  }
  else
    error_exit("Unexpected token in query definition.\nNot parsed: %s", str0);
  
  /*
  if (str_tok_cmp_adv_qd(str0, "ALLOCATE", 3))
  {
    str_eat_ws(str0);
    
    tmp_d = strtod(str0, &str1);
    str0 = str1;
    str_eat_ws(str0);
    if (str_tok_cmp_adv_qd(str0, "BLOCKS", 1) || str_tok_cmp_adv_qd(str0, "BLKS", 1))
      tmp_d *= GLOBALS::blk_size;
    else if (str_tok_cmp_adv_qd(str0, "B", 1))
      ;
    else if (str_tok_cmp_adv_qd(str0, "KB", 1))
      tmp_d *= (1 << 10);
    else if (str_tok_cmp_adv_qd(str0, "MB", 1))
      tmp_d *= (1 << 20);
    else if (str_tok_cmp_adv_qd(str0, "GB", 1))
      tmp_d *= (1 << 30);
    else
      error_exit("The amount of memory specified in query definition do not have a valid unit. "
        "Valid units are BLOCKS BLKS B KB MB GB");

    if (tmp_d < GLOBALS::blk_size || tmp_d > UINT_MAX)
      error_exit("The amount of memory specified in query definition is invalid");
    else
      QUERY::buffer_memory = uint4(tmp_d);

    str_eat_ws(str0);
    
    if (QUERY::query_type == 0)
      QUERY::traverse_memory = 0;
    else if (str_tok_cmp_adv_qd(str0, ",", 2))
    {    
      str_eat_ws(str0);
      
      tmp_d = strtod(str0, &str1);
      str0 = str1;
      str_eat_ws(str0);
      if (str_tok_cmp_adv_qd(str0, "B", 1))
        ;
      else if (str_tok_cmp_adv_qd(str0, "KB", 1))
        tmp_d *= (1 << 10);
      else if (str_tok_cmp_adv_qd(str0, "MB", 1))
        tmp_d *= (1 << 20);
      else if (str_tok_cmp_adv_qd(str0, "GB", 1))
        tmp_d *= (1 << 30);
      else
        error_exit("The amount of additional memory specified in query definition do not have a valid unit. "
        "Valid units are B KB MB GB");
      
      if ((QUERY::query_type < q_spath_tree_ && tmp_d < sizeof(bv_list_elem)) || 
        (QUERY::query_type == q_spath_tree_ && tmp_d < sizeof(bv_list_slt_start_with)) ||
        tmp_d > UINT_MAX)
        error_exit("The amount of additional memory specified in query definition is invalid");
      else
        QUERY::traverse_memory = uint4(tmp_d);

      str_eat_ws(str0); 
    } 
    else
      QUERY::traverse_memory = PARAM::query_memory;
  }
  else
  {*/
    //QUERY::buffer_memory = PARAM::query_memory;
    //QUERY::traverse_memory = QUERY::query_type == 0 ? 0 : PARAM::query_memory;
  /*}*/
  
  if (str_tok_cmp_adv_qd(str0, "SPOOL TO", 3))
  {
    str_eat_ws(str0);

    if (str_tok_cmp_adv_qd(str0, "\"",2))
    { 
      str1 = filename;
      while (*str0 != '\"')
      {
        if (*str0 == '\0') 
          error_exit("Unexpected end of query definition.");
        str_app_adv(str1, *str0++);
      }
      
      str0++;
      *str1 = '\0';

      QUERY::output_fp = fopen(filename, "w");
      if (QUERY::output_fp == NULL)
        error_exit("File '%s' specified for query output cannot be opened");
      QUERY::must_close_output = true;
    }
    else
      error_exit("Invalid stream specified for query output. Valid streams are stdout stderr \"filename\"");
  }
  else
  {
    QUERY::output_fp = stdout;
    QUERY::must_close_output = false;
  }

  str_eat_ws(str0);

  if (*str0 != ';')
    error_exit("Unexpected token in query definition.\nNot parsed: %s", str0);

  str_eat_ws_tok_sd(str0, ';');

  str_eat_ws(str0);

  //if(*str0 != '\0')
  //  print_ln("Warning: Found text after ';' in query definition");

  query_vertex();

  free(QUERY::sel_path);
  if (QUERY::cnt_sel_root > 0)
    free(QUERY::sel_root_arr[0]);

  if(*str0 != '\0')
    read_query(str0, false);

  
  if (first_q)
    free(line);
}
