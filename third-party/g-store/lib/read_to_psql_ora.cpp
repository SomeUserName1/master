#include "read_to_psql_ora.h"

#include "res_words_psql_ora.h"
#include "util_t.h"
#include "util.h"
#include "structs.h"
#include "parameters.h"

/* Implementation of EXPORT TO, see dissertation Figure 11 */
//#include <g-store.h>
//TODO:make it MAXLINE + 1 everywhere


void read_to_ora_postgres(FILE* fp_in, FILE* fp_out, bool ora)
{
  char* line_in = malloc1<char>(MAXLINE + 1, "todo");
  char* line_out = malloc1<char>(MAXLINE + 1, "todo");
  char* line_v1 = malloc1<char>(MAXLINE + 1, "todo");
  char* line_v2 = malloc1<char>(MAXLINE + 1, "todo");

  char* str0_in = line_in, *str0_out = line_out, *str0_v1 = line_v1, *str0_v2 = line_v2;
  char* str1;
  int adv = 0, progress, cnt_x = 0;
  int i, j, tmp;
  bool dupl;

  char* v1_begin;
  char* v2_begin;
  char* v2_begin2;
  ora ? str_app_adv(str0_out, "DROP TABLE ") : str_app_adv(str0_out, "DROP TABLE IF EXISTS ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, " CASCADE");
  str_app_adv(str0_out, ';');
  str_app_adv(str0_out, '\n');

  ora ? str_app_adv(str0_out, "DROP TABLE edges") : str_app_adv(str0_out, "DROP TABLE IF EXISTS edges");
  str_app_adv(str0_out, " CASCADE");
  str_app_adv(str0_out, ';');
  str_app_adv(str0_out, '\n');

  str_app_adv(str0_out, "CREATE TABLE ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, '(');
  
  str_app_adv(str0_v1, "INSERT INTO ");
  str_app_adv(str0_v1, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_v1, '(');

  if (GLOBALS::first_is_id == 0)
  {
    str_app_adv(str0_out, "orig_id integer PRIMARY KEY,");
    str_app_adv(str0_v1, "orig_id,");
  }


  for (i = 0; i < GLOBALS::num_fields; i++)
  {
    //mysterious errors in release mode if a switch statement is used here 
    if (GLOBALS::rec_fields[i].field_datatype == bool_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_v1, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      ora ? str_app_adv(str0_out, " char(1)") : str_app_adv(str0_out, " boolean");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == int_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_v1, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, ' ');
      if (GLOBALS::rec_fields[i].field_length == 1)
        str_app_adv(str0_out, " numeric(3)");
      else if (GLOBALS::rec_fields[i].field_length == 2)
        ora ? str_app_adv(str0_out, " numeric(5)") : str_app_adv(str0_out, " int2");
      else if (GLOBALS::rec_fields[i].field_length == 4)
        ora ? str_app_adv(str0_out, " numeric(10)") : str_app_adv(str0_out, " int4");
      else if (GLOBALS::rec_fields[i].field_length == 8)
        ora ? str_app_adv(str0_out, " numeric(20)") : str_app_adv(str0_out, " int8");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == uint_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_v1, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      if (GLOBALS::rec_fields[i].field_length == 1)
        str_app_adv(str0_out, " numeric(3)");
      else if (GLOBALS::rec_fields[i].field_length == 2)
        str_app_adv(str0_out, " numeric(5)");
      else if (GLOBALS::rec_fields[i].field_length == 4)
        str_app_adv(str0_out, " numeric(10)");
      else if (GLOBALS::rec_fields[i].field_length == 8)
        str_app_adv(str0_out, " numeric(20)");

      str_app_adv(str0_out, " check (");
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " >= 0)");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == double_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_v1, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      ora ? str_app_adv(str0_out, " numeric(16,8)") : str_app_adv(str0_out, " double precision");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == fixchar_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_v1, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " char(");
      inttostr_adv(str0_out, GLOBALS::rec_fields[i].field_length);
      str_app_adv(str0_out, ')');
    }
    else if (GLOBALS::rec_fields[i].field_datatype == varchar_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_v1, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      ora ? str_app_adv(str0_out, " varchar(1999)") : str_app_adv(str0_out, " text");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == skip_)
    {
      continue;
    }

    str_app_adv(str0_out, ',');
    str_app_adv(str0_v1, ',');

  }

  str0_out--;
  str0_v1--;

  str_app_adv(str0_out, ')');
  str_app_adv(str0_out, ';');
  str_app_adv(str0_out, '\n');

  str_app_adv(str0_v1, ") VALUES(");
  v1_begin = str0_v1;

  str_app_adv(str0_out, "CREATE TABLE edges");
  str_app_adv(str0_out, "(a integer, b integer);\n");
  
  str_app_adv(str0_out, "CREATE OR REPLACE VIEW graphbase AS SELECT * FROM ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, " JOIN edges ON ");
  if (GLOBALS::first_is_id == 0)
     str_app_adv(str0_out, "orig_id");
  else
     str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[0].field_name, psql_ora_reserved));
  str_app_adv(str0_out, " = a;\n");
  str_app_adv(str0_out, '\0');
  
  str_app_adv(str0_v2, "INSERT INTO edges");
  str_app_adv(str0_v2, "(a, b) VALUES(");
  v2_begin = str0_v2;

  fputs(line_out, fp_out);
  str0_out = line_out;

  do 
    fgets(line_in, MAXLINE, fp_in);
  while (*line_in == '#' && !feof(fp_in));

  progress = GLOBALS::num_vertices/20;

  printf("\nExporting vertices:     0 [                    ] %d", GLOBALS::num_vertices);
  put_mult_char('\b',len_num_str(GLOBALS::num_vertices) + 20 + 2);


  //////////////////////////////////////////////////////////////////////////
  // START READING VALUES

  std::vector<int> rem_dupl; //easier here that in SQL

  for (i = 0; i < GLOBALS::num_vertices; i++)
  {
    if (++adv == progress && (adv = 0) == 0 && cnt_x++ == cnt_x)
      putchar('=');

    fgets(line_in, MAXLINE, fp_in);

    if (strlen(line_in) == MAXLINE) 
      error_exit("read-buffer (%d char) exhausted", MAXLINE);

    str0_in = line_in;
    str0_v1 = v1_begin;
    str0_v2 = v2_begin;
    
    rem_dupl.clear();
    rem_dupl.push_back(strtol(str0_in,&str1,10));

    str_cpy_until_wsnl_adv(str0_in, str0_v2);
    str_app_adv(str0_v2, ',');
    str0_in = line_in;
    
    v2_begin2 = str0_v2;

    if (GLOBALS::first_is_id == 0)
    {
      str_cpy_until_wsnl_adv(str0_in, str0_v1);
      str_eat_ws(str0_in);
      str_app_adv(str0_v1, ',');
    }

    for (j = 0; j < GLOBALS::num_fields; j++)
    {
      if (GLOBALS::rec_fields[j].field_datatype == bool_)
      {
        if (*str0_in == '0' || *str0_in == 'F' || *str0_in == 'f')
          ora ? str_app_adv(str0_v1, "f") : str_app_adv(str0_v1, "FALSE");
        else //(*str0 == '1' || *str0 == 'T' || *str0 == 't')
          ora ? str_app_adv(str0_v1, "t") : str_app_adv(str0_v1, "TRUE");
        
        while (*str0_in != ' ' && *str0_in != '\n' && *str0_in != '\0')
          str0_in++;
        str_app_adv(str0_v1, ',');
      }
      else if (GLOBALS::rec_fields[j].field_datatype == int_
        || GLOBALS::rec_fields[j].field_datatype == uint_
        || GLOBALS::rec_fields[j].field_datatype == double_)
      {
        str_cpy_until_wsnl_adv(str0_in, str0_v1);
        str_app_adv(str0_v1, ',');
      }
      else if (GLOBALS::rec_fields[j].field_datatype == fixchar_
        || GLOBALS::rec_fields[j].field_datatype == varchar_)
      {
        str0_in++;
        str_app_adv(str0_v1, '\'');
        str1 = str0_v1;
        str_read_after_quote_adv(str0_in, str0_v1);
        while (str1 != str0_v1)
        {
          //the latter is needed to prevent ORA from interpreting the text following '&' as a variable
          if (*str1 == '\'' || *str1 == '&') 
            *str1 = '_';
          str1++;
        }
        str_app_adv(str0_v1, '\'');
        str_app_adv(str0_v1, ',');   
      }
      else if (GLOBALS::rec_fields[j].field_datatype == skip_)
        str_skip_adv(str0_in);

      str_eat_ws(str0_in);
    }
    str0_v1--;
    str_app_adv(str0_v1, ')');
    str_app_adv(str0_v1, ';');
    str_app_adv(str0_v1, '\n');
      
    fwrite(line_v1, str0_v1 - line_v1, 1, fp_out);

    for (;;) 
    {
      if (*str0_in=='\n' || *str0_in=='\0')
        break;
      
      tmp = strtol(str0_in,&str1,10);
      dupl = false;
      for (j = 0; j < (int) rem_dupl.size(); j++)
      {
        if (rem_dupl[j] == tmp)
        {
          while (*str0_in != ' ' && *str0_in != '\t' && *str0_in!='\n' && *str0_in!='\0')
            str0_in++;
          str_eat_ws(str0_in);
          dupl = true;
          break;
        }
      }
      
      if (dupl)
        continue;

      rem_dupl.push_back(tmp);

      str_cpy_until_wsnl_adv(str0_in, str0_v2);

      str_app_adv(str0_v2, ')');
      str_app_adv(str0_v2, ';');
      str_app_adv(str0_v2, '\n');
      
      fwrite(line_v2, str0_v2 - line_v2, 1, fp_out);

      str_eat_ws(str0_in);
      str0_v2 = v2_begin2;
    }
  }

  ora ? fputs("COMMIT;\n", fp_out) : 0;
  fputs("CREATE INDEX idx_a ON edges(a);\n", fp_out);
  fputs("CREATE INDEX idx_b ON edges(b);\n", fp_out);

  printf("\n\n");
  free_all(4, line_in, line_out, line_v1, line_v2);
  
  printf("SQL file has been written to %s\n", ora ? PARAM::default_ora_export_filename : PARAM::default_psql_export_filename);
  fclose(fp_out);
}

void read_to_postgres_copy(FILE* fp_in, FILE* fp_out, FILE* fp_out_v, FILE* fp_out_e)
{
  char* line_in = malloc1<char>(MAXLINE + 1, "todo"); 
  char* line_out = malloc1<char>(MAXLINE + 1, "todo"); 
  char* line_v1 = malloc1<char>(MAXLINE + 1, "todo");
  char* line_v2 = malloc1<char>(MAXLINE + 1, "todo");

  char delim = '\"';
  char* str0_in = line_in, *str0_out = line_out, *str0_v1 = line_v1, *str0_v2 = line_v2;
  char* str1;
  int adv = 0, progress, cnt_x = 0; 
  int i, j, tmp;
  bool dupl;
  char filename[260];

  char* v1_begin;
  char* v2_begin;
  char* v2_begin2;
  str_app_adv(str0_out, "DROP TABLE IF EXISTS ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, "CASCADE ;");
  str_app_adv(str0_out, '\n');

  str_app_adv(str0_out, "DROP TABLE IF EXISTS edges CASCADE");
  str_app_adv(str0_out, ';');
  str_app_adv(str0_out, '\n');

  str_app_adv(str0_out, "CREATE TABLE ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, '(');

  if (GLOBALS::first_is_id == 0)
  {
    str_app_adv(str0_out, "orig_id integer PRIMARY KEY,");
  }

  for (i = 0; i < GLOBALS::num_fields; i++)
  {
    //mysterious errors in release mode if a switch statement is used here 
    if (GLOBALS::rec_fields[i].field_datatype == bool_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " boolean");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == int_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, ' ');
      if (GLOBALS::rec_fields[i].field_length == 1)
        str_app_adv(str0_out, " numeric(3)");
      else if (GLOBALS::rec_fields[i].field_length == 2)
        str_app_adv(str0_out, " int2");
      else if (GLOBALS::rec_fields[i].field_length == 4)
        str_app_adv(str0_out, " int4");
      else if (GLOBALS::rec_fields[i].field_length == 8)
        str_app_adv(str0_out, " int8");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == uint_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      if (GLOBALS::rec_fields[i].field_length == 1)
        str_app_adv(str0_out, " numeric(3)");
      else if (GLOBALS::rec_fields[i].field_length == 2)
        str_app_adv(str0_out, " numeric(5)");
      else if (GLOBALS::rec_fields[i].field_length == 4)
        str_app_adv(str0_out, " numeric(10)");
      else if (GLOBALS::rec_fields[i].field_length == 8)
        str_app_adv(str0_out, " numeric(20)");

      str_app_adv(str0_out, " check (");
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " >= 0)");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == double_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " double precision");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == fixchar_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " char(");
      inttostr_adv(str0_out, GLOBALS::rec_fields[i].field_length);
      str_app_adv(str0_out, ')');
    }
    else if (GLOBALS::rec_fields[i].field_datatype == varchar_)
    {
      str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[i].field_name, psql_ora_reserved));
      str_app_adv(str0_out, " text");
    }
    else if (GLOBALS::rec_fields[i].field_datatype == skip_)
    {
      continue;
    }

    str_app_adv(str0_out, ',');
  }

  str0_out--;

  str_app_adv(str0_out, ')');
  str_app_adv(str0_out, ';');
  str_app_adv(str0_out, '\n');

  v1_begin = str0_v1;

  str_app_adv(str0_out, "CREATE TABLE edges");
  str_app_adv(str0_out, "(a integer, b integer);\n");
  
  str_app_adv(str0_out, "CREATE OR REPLACE VIEW graphbase AS SELECT * FROM ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, " JOIN edges ON ");
  if (GLOBALS::first_is_id == 0)
    str_app_adv(str0_out, "orig_id");
  else
    str_app_adv(str0_out, replace_if_reserved(GLOBALS::rec_fields[0].field_name, psql_ora_reserved));
  str_app_adv(str0_out, " = a;\n");

  str_app_adv(str0_out, "COPY ");
  str_app_adv(str0_out, replace_if_reserved(GLOBALS::graph_name, psql_ora_reserved));
  str_app_adv(str0_out, " FROM \'");
  strcat1(filename, PARAM::default_psql_export_filename, "_v");
  str_app_adv(str0_out, filename);
  str_app_adv(str0_out, "\' DELIMITER \'");
  str_app_adv(str0_out, delim);
  str_app_adv(str0_out, "\';\n");

  str_app_adv(str0_out, "COPY ");
  str_app_adv(str0_out, "edges");
  str_app_adv(str0_out, " FROM \'");
  strcat1(filename, PARAM::default_psql_export_filename, "_e");
  str_app_adv(str0_out, filename);
  str_app_adv(str0_out, "\' DELIMITER \'");
  str_app_adv(str0_out, delim);
  str_app_adv(str0_out, "\';\n");

  str_app_adv(str0_out, '\0');

  v2_begin = str0_v2;

  fputs(line_out, fp_out);
  str0_out = line_out;

  do 
  fgets(line_in, MAXLINE, fp_in);
  while (*line_in == '#' && !feof(fp_in));

  progress = GLOBALS::num_vertices/20;

  printf("\nExporting vertices:     0 [                    ] %d", GLOBALS::num_vertices);
  put_mult_char('\b',len_num_str(GLOBALS::num_vertices) + 20 + 2);

  //////////////////////////////////////////////////////////////////////////
  // START READING VALUES

  std::vector<int> rem_dupl; //easier here that in SQL

  for (i = 0; i < GLOBALS::num_vertices; i++)
  {
    if (++adv == progress && (adv = 0) == 0 && cnt_x++ == cnt_x)
      putchar('=');

    fgets(line_in, MAXLINE, fp_in);

    if (strlen(line_in) == MAXLINE) 
      error_exit("read-buffer (%d char) exhausted", MAXLINE);

    str0_in = line_in;
    str0_v1 = v1_begin;
    str0_v2 = v2_begin;

    rem_dupl.clear();
    rem_dupl.push_back(strtol(str0_in,&str1,10));

    str_cpy_until_wsnl_adv(str0_in, str0_v2);
    str_app_adv(str0_v2, delim);
    str0_in = line_in;

    v2_begin2 = str0_v2;

    if (GLOBALS::first_is_id == 0)
    {
      str_cpy_until_wsnl_adv(str0_in, str0_v1);
      str_eat_ws(str0_in);
      str_app_adv(str0_v1, delim);
    }

    for (j = 0; j < GLOBALS::num_fields; j++)
    {
      if (GLOBALS::rec_fields[j].field_datatype == bool_)
      {
        if (*str0_in == '0' || *str0_in == 'F' || *str0_in == 'f')
          str_app_adv(str0_v1, "FALSE");
        else //(*str0 == '1' || *str0 == 'T' || *str0 == 't')
          str_app_adv(str0_v1, "TRUE");

        while (*str0_in != ' ' && *str0_in != '\n' && *str0_in != '\0')
          str0_in++;
        str_app_adv(str0_v1, delim);
      }
      else if (GLOBALS::rec_fields[j].field_datatype == int_
        || GLOBALS::rec_fields[j].field_datatype == uint_
        || GLOBALS::rec_fields[j].field_datatype == double_)
      {
        str_cpy_until_wsnl_adv(str0_in, str0_v1);
        str_app_adv(str0_v1, delim);
      }
      else if (GLOBALS::rec_fields[j].field_datatype == fixchar_
        || GLOBALS::rec_fields[j].field_datatype == varchar_)
      {
        str0_in++;
        str1 = str0_v1;
        str_read_after_quote_adv(str0_in, str0_v1);
        while (str1 != str0_v1)
        {
          //the latter is needed to prevent ORA from interpreting the text following '&' as a variable
          if (*str1 == '\'' || *str1 == '&') 
            *str1 = '_';
          str1++;
        }
        str_app_adv(str0_v1, delim);   
      }
      else if (GLOBALS::rec_fields[j].field_datatype == skip_)
        str_skip_adv(str0_in);

      str_eat_ws(str0_in);
    }
    str0_v1--;
    str_app_adv(str0_v1, '\n');

    fwrite(line_v1, str0_v1 - line_v1, 1, fp_out_v);

    for (;;) 
    {
      if (*str0_in=='\n' || *str0_in=='\0')
        break;

      tmp = strtol(str0_in,&str1,10);
      dupl = false;
      for (j = 0; j < (int) rem_dupl.size(); j++)
      {
        if (rem_dupl[j] == tmp)
        {
          while (*str0_in != ' ' && *str0_in != '\t' && *str0_in!='\n' && *str0_in!='\0')
            str0_in++;
          str_eat_ws(str0_in);
          dupl = true;
          break;
        }
      }

      if (dupl)
        continue;

      rem_dupl.push_back(tmp);

      str_cpy_until_wsnl_adv(str0_in, str0_v2);

      str_app_adv(str0_v2, '\n');

      fwrite(line_v2, str0_v2 - line_v2, 1, fp_out_e);

      str_eat_ws(str0_in);
      str0_v2 = v2_begin2;
    }
  }

  fputs("CREATE INDEX idx_a ON edges(a);\n", fp_out);
  fputs("CREATE INDEX idx_b ON edges(b);\n", fp_out);

  printf("\n\n");
  free_all(4, line_in, line_out, line_v1, line_v2);

  printf("SQL files have been written to %s\n\n", PARAM::default_psql_export_filename);
  fclose(fp_out);
  fclose(fp_out_v);
  fclose(fp_out_e);
}
