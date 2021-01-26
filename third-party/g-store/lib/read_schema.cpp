#include "read_schema.h"

#include "defs.h"
#include "structs.h"
#include "util.h"
#include "util_t.h"
#include "memory_mgr.h"
#include "find_blocksize.h"
#include "entry_points.h"
#include "parameters.h"

/* Parser for G-Store schema definition. See dissertation Figure 11 */
//#include <g-store.h>

//todo: disallow WHERE, and other keywords;

char MY_NAME[];

void read_schema(char* line)
{
  char* str0 = line;
  char* str1;
  bool first_fld = true;
  int str_len = 0;
  char filename[256];
  FILE* fp_in, *fp_parts = NULL;
  char flat = 0;

  if (*GLOBALS::graph_name != '\0')
  {
    if (confirm_yn("You are about to overwrite graph '%s'. "
          "Are you sure you want to continue?", GLOBALS::graph_name))
    {
      mem_close_disks();
      mem_delete_disks();
      strcat1(filename, PARAM::working_dir, PARAM::globals_filename);
      remove(filename);
    }
    else
      return;
  }

  CreateDirectory(PARAM::working_dir, NULL);
  
  std::vector<rec_struct> rec_fields_tmp;
  rec_struct rs_elem;

  str_eat_ws(str0);

  str_tok_cmp_adv_sd(str0, "CREATE GRAPH", 4);
  
  str_eat_ws(str0);

  if (isdigit(*str0))
    error_exit("Graph name may not start with a digit");

  str_len = 0;
  str1 = GLOBALS::graph_name;
  while (!str_is_ws_sd(str0, '('))
  {
    str_app_adv(str1, *str0++);
    if(++str_len == 31)
      error_exit("maximum graph name length is 31 characters");
  }
  if (str_len == 0)
    error_exit("missing graph name");

  *str1 = '\0';
  str_eat_ws_tok_sd(str0, '(');
  
  for(;;)
  {
    str_eat_ws(str0);

    if (isdigit(*str0))
      error_exit("Field name may not start with a digit");

    str_len = 0;
    str1 = rs_elem.field_name;
    while (!str_is_ws_sd(str0))
    {
      str_app_adv(str1, *str0++);
      if (++str_len == 31)
        error_exit("maximum field title length is 31 characters");
    }

    if (str_len == 0)
      error_exit("missing field title");

    *str1 = '\0';

    str_eat_ws(str0);

    if (str_tok_cmp_adv_sd(str0, "BOOL", 1))
    {
      rs_elem.field_datatype = bool_;
      rs_elem.field_length = -1;
    }
    else if (str_tok_cmp_adv_sd(str0, "CHAR", 1))
    {
      rs_elem.field_datatype = fixchar_;
      str_eat_ws_tok_err(str0, '(', "schema definition");
      rs_elem.field_length = strtol(str0, &str1, 10);
      str0 = str1;
      str_eat_ws_tok_err(str0, ')', "schema definition");
    }
    else if (str_tok_cmp_adv_sd(str0, "VARCHAR", 1))
    {
      rs_elem.field_datatype = varchar_;
      rs_elem.field_length = -1;
    }
    else if (str_tok_cmp_adv_sd(str0, "INT", 1))
    {
      rs_elem.field_datatype = int_;
      str_eat_ws_tok_sd(str0, '(');
      rs_elem.field_length = strtol(str0, &str1, 10);
      str0 = str1;
      str_eat_ws_tok_sd(str0, ')');
    }
    else if (str_tok_cmp_adv_sd(str0, "UINT", 1))
    {
      rs_elem.field_datatype = uint_;
      str_eat_ws_tok_sd(str0, '(');
      rs_elem.field_length = strtol(str0, &str1, 10);
      str0 = str1;
      str_eat_ws_tok_sd(str0, ')');
    }
    else if (str_tok_cmp_adv_sd(str0, "DOUBLE", 1))
    {
      rs_elem.field_datatype = double_;
      rs_elem.field_length = -1;
    }
    else if (str_tok_cmp_adv_sd(str0, "SKIP", 1))
    {
      rs_elem.field_datatype = skip_;
      rs_elem.field_length = -1;
    }
    else if (*str0 == ')')
      error_exit("Unexpected ')' in schema definition.\nNot parsed: %s", str0);
    else
      error_exit("Unknown data type in schema definition - "
        "please refer to documentation.\nNot parsed: %s", str0);
    
    rec_fields_tmp.push_back(rs_elem);

    if (first_fld)
    {
      str_eat_ws(str0);
      first_fld = false;
      
      if (str_tok_cmp_adv_sd(str0, "IS ID", 1))
        GLOBALS::first_is_id = 1;
      else
        GLOBALS::first_is_id = 0;
    }

    str_eat_ws(str0);

    if (*str0 == ')')
    {
      str0++;
      break;
    }

    str_eat_ws_tok_sd(str0, ',');
  }
  
  str_eat_ws(str0);

  ////////////////////////////////////////////////////////////////////////// FROM
  str_tok_cmp_adv_sd(str0, "FROM", 4);
  str_eat_ws_tok_sd(str0, '\"');
  
  str1 = filename;
  while(*str0 != '\"')
  {
    if (*str0 == ';')
      error_exit("Unexpected end in schema definition. Expected \"");
    if (str1 - filename >= 255)
      error_exit("Graph input file name to long in schema definition");
    *str1++ = *str0++;
  }
  *str1 = '\0';
  str0++;

  fp_in = fopen(filename,"r");
  if (fp_in == NULL) 
    error_exit("Error opening graph input file '%s' specified in schema definition.", filename);
  
  strcpy(GLOBALS::graph_input_filename, filename);

  str_eat_ws(str0);
  
  ////////////////////////////////////////////////////////////////////////// BASED ON
  if (str_tok_cmp_adv_sd(str0, "BASED ON", 3))
  {
    str_eat_ws_tok_sd(str0, '\"');
    str1 = filename;

    while(*str0 != '\"')
    {
      if (*str0 == ';')
        error_exit("Unexpected end in schema definition. Expected \"");
      if (str1 - filename >= 255)
        error_exit("Partition file name to long in schema definition");
      *str1++ = *str0++;
    }
    *str1 = '\0';
    str0++;

    fp_parts = fopen(filename, "r");
    if (fp_parts == NULL) 
      error_exit("Error opening partition input file '%s' specified in schema definition.", filename);
  } 
  ////////////////////////////////////////////////////////////////////////// BASED ON
  else if (str_tok_cmp_adv_sd(str0, "FLAT", 3))
  {
    str_eat_ws(str0);
    if (str_tok_cmp_adv_sd(str0, "RANDOM", 1))
      flat = 1;
    else if (str_tok_cmp_adv_sd(str0, "STRAIGHT", 1))
      flat = 2;
    else 
      error_exit("Expected RANDOM or STRAIGHT after FLAT in schema definition.");
  }
    
  str_eat_ws_tok_sd(str0, ';');
  
  while (*str0 == ' ' || *str0 == '\n' || *str0 == '\t')
    str0++;

  if(*str0 != '\0')
    print_ln("Warning: Found text after ';' in schema definition");
  
  GLOBALS::num_fields = rec_fields_tmp.size();

  GLOBALS::rec_fields = (rec_struct*) malloc_b(GLOBALS::num_fields * sizeof(rec_struct), "todo");
    
  memcpy(GLOBALS::rec_fields, &rec_fields_tmp[0], GLOBALS::num_fields * sizeof(rec_struct));

  free(line);

  determine_blocksize(true);
  
  LARGE_INTEGER start, finish, freq;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  if (flat > 0 || fp_parts != NULL)
    create_db_part(fp_in, fp_parts, flat);
  else
    create_db_new(fp_in);
  
  QueryPerformanceCounter(&finish);
  printf("Execution time : %.3fs\n\n", ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart));

  if (*MY_NAME != '\0')         
    WinExec(MY_NAME, SW_SHOW);    //To be depreciated.

  exit(EXIT_SUCCESS);
}

void assign_rec_field_fkts()
{
  bool first_var_fld = true;

  for (int i = 0; i < GLOBALS::num_fields; i++)
  {
    GLOBALS::rec_fields[i].h_fix_off1 = GLOBALS::h_fix_off[i];
    GLOBALS::rec_fields[i].h_var_fld1 = GLOBALS::h_var_fld[i];

    switch (GLOBALS::rec_fields[i].field_datatype)
    {
    case bool_:
      GLOBALS::rec_fields[i].append_to_str = typetostr_adv<bool>;
      GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_num_bool;
      break;

    case uint_:
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<uint1>;
        break;
      case 2:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<uint2>;
        break;
      case 4:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<uint4>;
        break;
      case 8:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<uint8>;
        break;
      }
      GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_num_bool;
      break;

    case int_:         
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<int1>;
        break;
      case 2:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<int2>;
        break;
      case 4:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<int4>;
        break;
      case 8:
        GLOBALS::rec_fields[i].append_to_str = typetostr_adv<int8>;
        break;
      }
      GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_num_bool;
      break;

    case double_:
      GLOBALS::rec_fields[i].append_to_str = typetostr_adv<double>;
      GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_num_bool;
      break;

    case fixchar_:
      GLOBALS::rec_fields[i].append_to_str = typetostr_adv<char*>;
      GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_fixchar;
      break;

    case varchar_:
      GLOBALS::rec_fields[i].append_to_str = typetostr_adv<char*>;
      if (first_var_fld)
      {
        GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_varchar_1;
        first_var_fld = false;
      }
      else
        GLOBALS::rec_fields[i].eff_append_to_str = &rec_struct::eff_append_to_str_varchar_n;

      break;
    }
  }
}
