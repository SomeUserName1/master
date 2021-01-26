#include "cmd_input.h"

#include "util_t.h"
#include "util.h"
#include "read_query.h"
#include "read_schema.h"
#include "read_to_psql_ora.h"
#include "structs.h"
#include "memory_mgr.h"
#include "parameters.h"

/* G-Store command line interface and parser */
//#include <g-store.h>

//todo: eliminate choice if db was created

bool FIRST_RUN;
void read_menu()
{
  char* line = malloc1<char>(MAXLINE, "todo");
  int i = 0;
  
  printf("G-Store>");
  fflush(stdin);

  for(;;)
  {  
    line[i] = getchar();
    
    if (line[i] == ';')
    {
      line[i] = ' ';    // fix currently needed in query parsing
      line[++i] = ';';
      line[++i]= '\0';
      break;
    }
    if (line[i] == '\n')
    {
      printf("G-Store> ");
      line[i] = ' ';
    }
    i++;
  } 
  
  printf("\n");
  evaluate_menu(line);
}


void evaluate_menu(char* line)
{
  char* str0 = line;
  
  str_eat_ws(str0);
  
  ////////////////////////////////////////////////////////////////////////// SELECT
  if (str_tok_cmp_adv_menu(str0, "SELECT", 2))
  {
    read_query(line);
    //line freed in read_query
  }
  ////////////////////////////////////////////////////////////////////////// CREATE
  else if (str_tok_cmp_adv_menu(str0, "CREATE", 2))
  {
    read_schema(line);
    //line freed in read_schema
  }
  ////////////////////////////////////////////////////////////////////////// EXPORT TO
  else if (str_tok_cmp_adv_menu(str0, "EXPORT TO", 3))
  {
    if (*GLOBALS::graph_name == '\0')
      error_exit("EXPORT TO can only be performed after a graph has been loaded.");

    bool ora;
    FILE* fp_in, *fp_out, *fp_out_v = NULL, *fp_out_e = NULL;

    str_eat_ws(str0);

    if (str_tok_cmp_adv_menu(str0, "PSQLCOPY", 3))
    {
      fp_out = fopen(PARAM::default_psql_export_filename, "w");

      if (fp_out == NULL)
        error_exit("Could not open PostgreSQL output file '%s'", PARAM::default_psql_export_filename);

      char filename[260];
      strcat1(filename, PARAM::default_psql_export_filename, "_v");
      fp_out_v = fopen(filename, "w");
      
      if (fp_out_v == NULL)
        error_exit("Could not open vertex output file '%s'", filename);

      strcat1(filename, PARAM::default_psql_export_filename, "_e");
      fp_out_e = fopen(filename, "w");

      if (fp_out_e == NULL)
        error_exit("Could not open edge output file '%s'", filename);
    }
    else if (str_tok_cmp_adv_menu(str0, "PSQL", 3))
    {
      ora = false;
      fp_out = fopen(PARAM::default_psql_export_filename, "w");
      if (fp_out == NULL)
        error_exit("Could not open PostgreSQL output file '%s'", PARAM::default_psql_export_filename);
      
    }
    else if (str_tok_cmp_adv_menu(str0, "ORA", 3))
    {
      ora = true;
      fp_out = fopen(PARAM::default_ora_export_filename, "w");
      if (fp_out == NULL)
        error_exit("Could not open Oracle output file '%s'", PARAM::default_ora_export_filename); 
    } 
    else
      error_exit("Expected PSQL or ORA.\nNot parsed: %s", str0);

    str_eat_ws(str0);

    if (str_tok_cmp_adv_menu(str0, "FROM", 3))
    {
      str_eat_ws_tok_menu(str0, '\"');
      char filename[256];
      char* str1 = filename;
      while(*str0 != '\"')
      {
        if (*str0 == ';')
          error_exit("Unexpected end in menu input. Expected \"");
        if (str1 - filename >= 255)
          error_exit("File name to long in menu input.");
        *str1++ = *str0++;
      }
      *str1 = '\0';
      str0++;

      fp_in = fopen(filename,"r");
      if (fp_in == NULL) 
        error_exit("Error opening file '%s' specified in menu input.", filename);
    }
    else
    {
      fp_in = fopen(GLOBALS::graph_input_filename,"r");
      if (fp_in == NULL) 
        error_exit("Error opening file '%s'. Use FROM keyword to specify a different path.", GLOBALS::graph_input_filename);
    }
    
    str_eat_ws_tok_menu(str0, ';');
    free(line);
    if (fp_out_v != NULL)
      read_to_postgres_copy(fp_in, fp_out, fp_out_v, fp_out_e);
    else
      read_to_ora_postgres(fp_in, fp_out, ora);
  }
  ////////////////////////////////////////////////////////////////////////// RUN
  else if (str_tok_cmp_adv_menu(str0, "RUN", 3))
  {
    str_eat_ws_tok_menu(str0, '\"');
    char filename[256];
    char* str1 = filename;
    while(*str0 != '\"')
    {
      if (*str0 == ';')
        error_exit("Unexpected end in menu input. Expected \"");
      if (str1 - filename >= 255)
        error_exit("File name to long in menu input.");
      *str1++ = *str0++;
    }
    *str1 = '\0';
    str0++;

    FILE* fp = fopen(filename,"r");
    if (fp == NULL) 
      error_exit("Error opening file '%s' specified in menu input.", filename);
   
    str_eat_ws_tok_menu(str0, ';');
    free(line);
    read_file(fp); //closes fp
  }
  ////////////////////////////////////////////////////////////////////////// 0
  else if (*str0 == '0')
  {
    str0++;
    str_eat_ws_tok_menu(str0, ';');

    if (*PARAM::null_filename == '\0')
      error_exit("no default filename specified in parameter file");
    
    FILE* fp = fopen(PARAM::null_filename,"r");
    if (fp == NULL) 
      error_exit("Error opening file '%s' from menu input.", PARAM::null_filename);

    free(line);
    
    //////////////////////////////////////////////////////////////////////////
    read_file(fp);
  }
  /*else if (*str0 == '1')
  {
    str0++;
    str_eat_ws_tok_menu(str0, ';');

    if (*PARAM::null_filename == '\0')
      error_exit("no default filename specified in parameter file");

    FILE* fp = fopen(PARAM::null_filename,"r");
    if (fp == NULL) 
      error_exit("Error opening file '%s' from menu input.", PARAM::null_filename);

    free(line);


    PARAM::query_memory = 33554432;
    rewind(fp); read_file(fp);
    PARAM::query_memory = 67108864;
    rewind(fp); read_file(fp);
    PARAM::query_memory = 134217728;
    rewind(fp); read_file(fp);
    PARAM::query_memory = 268435456;
    rewind(fp); read_file(fp);
  }*/
  ////////////////////////////////////////////////////////////////////////// EXIT
  else if (str_tok_cmp_adv_menu(str0, "EXIT", 1))
  {
    printf("Goodbye!\n");
    exit(EXIT_SUCCESS);
  }
  ////////////////////////////////////////////////////////////////////////// HELP
  else if (str_tok_cmp_adv_menu(str0, "HELP", 1))
  {
    free(line);
    display_help();
  }
  ////////////////////////////////////////////////////////////////////////// DESCRIBE
  else if (str_tok_cmp_adv_menu(str0, "DESCRIBE", 1))
  {
    free(line);
    display_describe();  
  }
  else
    error_exit("Unknown menu input. Refer to documentation or run HELP;");
}


void read_file(FILE* fp)
{
  char* line1 = malloc1<char>(MAXLINE, "todo");
  char* line = malloc1<char>(MAXLINE, "todo");
  char* str0 = line;
  int line1_len, str_len = 0;

  while(!feof(fp))
  {
    if (fgets(line1, MAXLINE, fp) == NULL)
      break;  // nothing read (e.g., empty line) and EOF
          
    if (*line1 == '#' || *line1 == '\n')
      continue;

    line1_len = strlen(line1);
    str_len += line1_len;
    if (str_len >= MAXLINE)
      error_exit("read buffer exhausted while reading input file");

    strcpy(str0, line1);
    str0 += (line1_len - 1);
    if (*str0 != '\n')
      str0++;
    str_app_adv(str0, ' ');
  }
  free(line1);
  fclose(fp);
  *str0 = '\0';
  str0 = line;

  str_eat_ws(str0);
  
  if (str_tok_cmp_adv_menu(str0, "SELECT", 2))
    read_query(line);  //frees line, 
  else if (str_tok_cmp_adv_menu(str0, "CREATE", 2))
    read_schema(line); //frees line
  else
    error_exit("Expecting SELECT or CREATE at the start of the file.\nNot parsed: %s", str0);
}

void display_describe()
{
  if (*GLOBALS::graph_name == '\0')
    error_exit("DESCRIBE can only be performed after a graph has been loaded.");

  
  print_ln("Graph name: %s", GLOBALS::graph_name);
  print_ln("Number of vertices: %d", GLOBALS::num_vertices);
  print_ln("Number of edges: %d\n", GLOBALS::num_edges);

  print_ln("%3s %-15s %s", "#", "Name", "Data type");
  print_ln("-----------------------------");

  for (int i = 0; i < GLOBALS::num_fields; i++)
  {
    printf("%3d %-15s ", i, GLOBALS::rec_fields[i].field_name);
    switch (GLOBALS::rec_fields[i].field_datatype)
    {
    case bool_:
      print_ln("BOOL");
      break;

    case int_:
      print_ln("INT(%d)", GLOBALS::rec_fields[i].field_length);
      break;

    case uint_:         
      print_ln("UINT(%d)", GLOBALS::rec_fields[i].field_length);
      break;

    case double_:
      print_ln("DOUBLE");
      break;

    case fixchar_:
      print_ln("CHAR(%d)", GLOBALS::rec_fields[i].field_length);
      break;

    case varchar_:
      print_ln("VARCHAR");
      break;

    case skip_:
      print_ln("SKIP");
      break;
    }
  }
  printf("\n\n");
  mem_print_disks_info();

}


//////////////////////////////////////////////////////////////////////////

void display_help()
{
  printf("G-Store is case-sensitive. You are currently in the menu.\n");
  printf("\n");
  printf("menu = \n");
  printf("     (schema_def | query_def) \n");
  printf("     | (DESCRIBE | RUN \'\"filename\"\' | \"0\"  \n");
  printf("        | \"EXPORT TO\" (PSQL | PSQLCOPY | ORA) [FROM \'\"filename\"\']  \n");
  printf("        | HELP | EXIT ) \";\".\n");
  printf("\n");
  printf("schema_def = \n");
  printf("     \"CREATE GRAPH\" graph_name \n");
  printf("     \"(\" (l_name l_type [\"IS ID\"]) {\",\" l_name l_type} \")\" \n");
  printf("     FROM \'\"filename\"\' [\"BASED ON\" \'\"filename\"\' \n");
  printf("                        | FLAT (RANDOM | STRAIGHT)] \";\". \n");
  printf("\n");
  printf("query_def = \n");
  printf("     SELECT select_list \n");
  printf("     ([WHERE cond] [traverse_clause] | path_clause [WHERE cond]) \n");
  printf("     [\"SPOOL TO\" \'\"filename\"\'] \";\". \n");
  printf("\n");
  printf("select_list = \n");
  printf("     \"*\" | (label | pseudol_a | pseudol_b | pseudol_c) [AS l_alias] \n");
  printf("     {\",\" (label | pseudol_a | pseudol_b | pseudol_c) [AS l_alias]}. \n");
  printf("\n");
  printf("traverse_clause = \n");
  printf("     \"START WITH\" (ANY | cond) [NOCYCLE]. \n");
  printf("\n");
  printf("path_clause = \n");
  printf("     (((\"IN PATH\" | \"IN SHORTEST PATH\") \"START WITH\" cond \"END WITH\" cond) \n");
  printf("     | ((\"IN SHORTEST PATH TREE\") \"START WITH\" cond)) \n");
  printf("     [THROUGH (cond | SEQUENCE (cond | ANY) {\".\" (cond | ANY)} ]. \n");
  printf("\n");
  printf("label = \n");
  printf("     l_name | l_num.\n");
  printf("\n");
  printf("pseudol_a = \n");
  printf("     GID | COUNT_EDGES | ISLEAF.\n");
  printf("\n");
  printf("pseudol_b = \n");
  printf("     ROWNUM | LEVEL | ISCYCLE.\n");
  printf("\n");
  printf("pseudol_c = \n");
  printf("     LPAD \"(\" \"\'char\'\" \",\" LEVEL \")\" \"||\" label \n");
  printf("     | PATH \"(\" label \",\"  \"\'char\'\" \")\" \n");
  printf("     | SOURCE \"(\" label \")\". \n");
  printf("\n");
  printf("cond = \n");
  printf("     ((label | pseudol_a | pseudol_b) \n");
  printf("      (\"=\" | \"!=\" | \"<=\" | \"<\" | \">=\" | \">\") expr) \n");
  printf("     | logical_cond. \n");
  printf("\n");
}

//////////////////////////////////////////////////////////////////////////

void read_argument(char* argv)
{
  FILE* fp = fopen(argv, "r");
  if (fp == NULL)
    error_exit("Cannot open file '%s' supplied as argument", argv);

  read_file(fp);
};