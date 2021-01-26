#include "read_input.h"

#include "structs.h"
#include "util.h"
#include "util_t.h"

/* See dissertation Section 6.2 */
//#include <g-store.h>

void read_graph(graph1* graph, FILE* fp_in)
{
  char* str0, *str1;
  int edge, me, adv = 0, progress; 
  int i, j, k, tmp, cnt_x = 0;
  bool dupl;
  int num_v, num_e;
  int cnt_comments = 0;
  
  char* line = malloc1<char>(MAXLINE, "todo");

  assert (fp_in != NULL);

  do 
  {
    fgets(line, MAXLINE, fp_in);
    cnt_comments++;
  }
  while (*line == '#' && !feof(fp_in));

  if (feof(fp_in))
    error_exit("graph input file empty");

  sscanf(line, "%d %d", &num_v, &num_e);

  GLOBALS::num_vertices = num_v;
  GLOBALS::num_edges = num_e;

  int* offset_e = malloc1<int>(num_v + 1, "graph->offset_e in read_graph1()");
  int* e_to     = malloc1<int>(num_e*2, "graph->e_to in read_graph1()");
  int* e_w      = malloc1_set<int>(num_e*2, 1, "read_graph: e_w");
  int* v_w      = malloc1_set<int>(num_v, 0, "graph->v_w in read_graph1()");
  int* v_inc    = malloc1_set<int>(num_v + 1, 0, "graph->v_inc in read_graph1()");

  printf("\nReading vertices:     0 [                    ] %d", num_v);
  put_mult_char('\b',len_num_str(num_v) + 20 + 2);
  
  progress = num_v/20;

  offset_e[0] = 0;

  for (i = 0, k = 0; i < num_v; i++)
  {
    fgets(line, MAXLINE, fp_in);
    if (strlen(line) == MAXLINE) 
    {
      print_endln();
      error_exit("read-buffer (%d char) exhausted", MAXLINE);
    }

    me = int(strtol(line, &str0, 10));
    if (me != i)
    {
      print_endln();
      error_exit("expecting vertex id %d at line %d, but found vertex id %d", i, i+cnt_comments, me);
    }
    
    if (GLOBALS::first_is_id == 1)
      str0 = line;

    v_w[i] = get_rec_len(str0, i + 1 + cnt_comments) + GLOBALS::header_len;

    tmp = k;

    for (;;) 
    {
      str_eat_ws(str0);

      if (*str0=='\n' || *str0=='\0')
        break;

      if (!(*str0 >= '0' && *str0 <= '9'))
      { print_endln();
        error_exit("line %d: expected edge list to start - instead found character %c", i + 1 + cnt_comments, *str0);}

      edge = strtol(str0, &str1, 10);
      str0 = str1;

      if (edge >= num_v)
        error_exit("line %d: edge %d out of range", i + 1 + cnt_comments, edge);

      //check for duplicate edges in edgelist
      if (edge == me)
      {
        putchar('\r');
        put_mult_char(' ', 55);
        printf("\rWarning: ignoring self-edge: %d -> %d\n\n", me, me);
        printf("Reading vertices:     0 [");
        put_mult_char('=', cnt_x);
        put_mult_char(' ', 20-cnt_x);
        printf("] %d", num_v);
        put_mult_char('\b',len_num_str(num_v) + 20 - cnt_x + 2);
        num_e--;
        continue;
      }

      for (j = tmp, dupl = false; j < k; j++)
        if (e_to[j] == edge)
        {
          putchar('\r');
          put_mult_char(' ', 55);
          printf("\rWarning: ignoring duplicate edge: %d -> %d\n\n", i, edge);
          printf("Reading vertices:     0 [");
          put_mult_char('=', cnt_x);
          put_mult_char(' ', 20-cnt_x);
          printf("] %d", num_v);
          put_mult_char('\b',len_num_str(num_v) + 20 - cnt_x + 2);
          num_e--;
          dupl = true;
          break; 
        }

      if (dupl)
        continue;

      if (edge < i)
        for (j = offset_e[edge]; j < offset_e[edge+1]; j++)
          if (e_to[j]==i)
          {
            e_w[j] = e_w[k] = 2;
            v_inc[edge + 1]--;
            v_inc[i + 1]--;
            num_e--;
            break;
          }        

      e_to[k++] = edge;
      v_inc[edge + 1]++;
    } 

    offset_e[i+1] = k;

    v_w[i] += 4 * (k - offset_e[i]);

    if (v_w[i] > GLOBALS::blk_writable) {
      print_endln();
      error_exit("line %d: G-Store does not currently support records spanning pages " 
                 "(record size: %d, maximum size: %d)", i + 1 + cnt_comments, v_w[i], GLOBALS::blk_size - 8);
    }

    if (++adv == progress && (adv = 0) == 0 && cnt_x++ == cnt_x)
      putchar('=');
  }

  num_e *= 2;

  e_to     = realloc1<int>(e_to, num_e, "todo913");
  e_w      = realloc1<int>(e_w, num_e, "todo346");

  int* org_size = malloc1<int>(num_v, "read_graph: e_w");

  for (i=1; i<num_v+1; i++)
    v_inc[i] += v_inc[i-1];

  for (i = num_v-1; i >= 0; i--)
  {
    tmp = offset_e[i+1] - offset_e[i];
    memmove( e_to + offset_e[i] + v_inc[i], e_to + offset_e[i], tmp * sizeof(int) );
    memmove( e_w  + offset_e[i] + v_inc[i], e_w  + offset_e[i], tmp * sizeof(int) );
    for (j = offset_e[i] + v_inc[i] + tmp; j < offset_e[i] + tmp + v_inc[i+1] ; j++)
    {
      e_to[j] = -1;
      e_w[j]  = 1;
    }
    org_size[i] = tmp;
  }

  for (i = 1; i < num_v + 1; i++)
    offset_e[i] = offset_e[i] + v_inc[i];

  for (i = 0; i < num_v; i++)
    for (j = offset_e[i]; j < offset_e[i] + org_size[i]; j++)
    {
      if (e_w[j] == 2)
        continue;

      k = offset_e[e_to[j]] + org_size[e_to[j]];
      while (e_to[k] != -1)
        k++;
      e_to[k] = i;
    }

  printf("\n\n");

  graph->c_level = 0;
  graph->num_v = num_v;
  graph->num_e = num_e;
  graph->offset_e = offset_e;
  graph->e_to = e_to;
  graph->v_w = v_w;
  graph->e_w = e_w;

  GLOBALS::fp_org_size = tmpfile();
  if (GLOBALS::fp_org_size == NULL)
    error_exit("Error creating a temporary file. Note that G-Store currently requires administrator privileges to run.");

  fwrite(org_size, sizeof(int), num_v, GLOBALS::fp_org_size);
  fseek(GLOBALS::fp_org_size, 0, SEEK_SET);

  free_all(3, line, v_inc, org_size);
}

//////////////////////////////////////////////////////////////////////////

void read_parts_into_graph(graph1* graph, FILE* fp_parts)
{
  int num_v = graph->num_v;
  int* v_w = graph->v_w;
  int i;
  int* part = malloc1<int>(num_v,"todo");
  int num_p = 0;

  char* line = malloc1<char>(MAXLINE + 1,"todo");

  assert (fp_parts != NULL);

  for (i = 0; i < num_v; i++)
  {
    fgets(line, MAXLINE, fp_parts);
    part[i] = strtol(line, NULL, 10);
    if (part[i] > num_p)
      num_p = part[i];
  }
  num_p++;

  int* p_w = malloc1_set<int>(num_p, 0, "todo");

  for (i = 0; i < num_v; i++)
    p_w[part[i]] += v_w[i];

  graph->num_p = num_p;
  graph->part = part;
  graph->p_w = p_w;

  fclose(fp_parts);
  free(line);
}


void generate_parts_into_graph(graph1* graph)
{
  int num_v = graph->num_v;
  int* v_w = graph->v_w;
  int i;
  int* part = malloc1<int>(num_v,"todo");
  int num_p = 0;
  int cnt_v_w = 0;

  for (i = 0; i < num_v; i++)
    if (cnt_v_w + v_w[i] <= GLOBALS::blk_writable)
    {
      part[i] = num_p;
      cnt_v_w += v_w[i];
    }
    else
    {
      part[i] = ++num_p;
      cnt_v_w = v_w[i];
    }

  num_p++;

  int* p_w = malloc1_set<int>(num_p, 0, "todo");

  for (i = 0; i < num_v; i++)
    p_w[part[i]] += v_w[i];

  graph->num_p = num_p;
  graph->part = part;
  graph->p_w = p_w;

}

//////////////////////////////////////////////////////////////////////////

void generate_rnd_parts_into_graph(graph1* graph)
{
  int num_v = graph->num_v;
  int* v_w = graph->v_w;
  int i,j;
  int* part = malloc1<int>(num_v,"todo");
  int num_p = 0;
  int cnt_v_w = 0;

  int* perm = malloc1<int>(num_v, "todo");
  for (i = 0; i < num_v; i++)
    perm[i] = i;
  permute(perm, num_v);

  for (j = 0; j < num_v; j++)
  {
    i = perm[j];
    if (cnt_v_w + v_w[i] <= GLOBALS::blk_writable)
    {
      part[i] = num_p;
      cnt_v_w += v_w[i];
    }
    else
    {
      part[i] = ++num_p;
      cnt_v_w = v_w[i];
    }
  }

    num_p++;

    int* p_w = malloc1_set<int>(num_p, 0, "todo");

    for (i = 0; i < num_v; i++)
      p_w[part[i]] += v_w[i];

    graph->num_p = num_p;
    graph->part = part;
    graph->p_w = p_w;

}


//////////////////////////////////////////////////////////////////////////

int get_rec_len(char* &str0, int line_id)
{
  uint8 tmp_uint8;
  int8  tmp_int8;
  double tmp_d;

  int tmp, cnt_len = 0;
  char* str1;

  int i;

  for (i = 0; i < GLOBALS::num_fields; i++)
  {
    str_eat_ws(str0);

    switch (GLOBALS::rec_fields[i].field_datatype)
    {
    case bool_:
      if (*str0 == '0' || *str0 == '1' || *str0 == 'T' || *str0 == 't' || *str0 == 'F' || *str0 == 'f')
        while (*str0 != ' ' && *str0 != '\n' && *str0 != '\0')
          str0++;
      else
        error_exit("in line %d: expected a bool value that started with 0 1 T t F f", line_id);
      cnt_len++;
      break;

    case int_:
      tmp_int8 = _strtoi64(str0, &str1, 10);
      str0 = str1;

      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        if (tmp_int8 < INT1_MIN || tmp_int8 > INT1_MAX)
          error_exit("in line %d: out of bounds int1 %lld", line_id, tmp_int8);
        cnt_len++;
        break;

      case 2:
        if (tmp_int8 < INT2_MIN || tmp_int8 > INT2_MAX)
          error_exit("in line %d: out of bounds int2 %lld", line_id, tmp_int8);
        cnt_len += 2;
        break;

      case 4:
        if (tmp_int8 < INT4_MIN  || tmp_int8 > INT4_MAX)
          error_exit("in line %d: out of bounds int4 %lld", line_id, tmp_int8);
        cnt_len += 4;
        break;

      case 8:
        cnt_len += 8;
        break;
      }
      str0 = str1;
      break;

    case uint_: 
      if (*str0 == '-')
        error_exit("in line %d: uint may not be negative", line_id);
      tmp_uint8 = _strtoui64(str0, &str1, 10);
      str0 = str1;

      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        if (tmp_uint8 > UINT1_MAX)
          error_exit("in line %d: out of bounds uint1 %llu", line_id, tmp_uint8);
        cnt_len++;
        break;

      case 2:
        if (tmp_uint8 > UINT2_MAX)
          error_exit("in line %d: out of bounds uint2 %llu", line_id, tmp_uint8);
        cnt_len += 2;
        break;

      case 4:
        if (tmp_uint8 > UINT4_MAX)
          error_exit("in line %d: out of bounds uint4 %llu", line_id, tmp_uint8);
        cnt_len += 4;
        break;

      case 8:
        cnt_len += 8;
        break;
      }

      break;

    case double_:
      tmp_d = strtod(str0, &str1);
      str0 = str1;
      cnt_len += 8;
      break;

    case fixchar_:
    case varchar_:
      tmp = cnt_len;
      if (*str0++ != '\"')
        error_exit("line %d: fixchar/varchar must be enclosed with \" characters", line_id);

      for (;;) 
      {
        if (*str0 == '\n' || *str0 == '\0')
          error_exit("line %d: fixchar/varchar must be enclosed with \" characters", line_id);
        else if (*str0 == '\\')
        {
          str0++;
          if (*str0 == '\"' || *str0 == '\\')
            str0++;
          cnt_len++;
        }
        else if (*str0 == '\"')
        {
          str0++;
          break;
        }
        else
        {
          str0++;
          cnt_len++;
        }
      }

      if (GLOBALS::rec_fields[i].field_datatype == fixchar_)
      {
        tmp = cnt_len - tmp;
        if (tmp > GLOBALS::rec_fields[i].field_length)
          error_exit("line %d: fixchar too large (found %d character, "
          "maximum field_length is %d)", line_id, tmp, GLOBALS::rec_fields[i].field_length);
        else
          cnt_len += (GLOBALS::rec_fields[i].field_length - tmp);
      }
      break;

    case skip_:
      if (*str0 == '\"')
      {
        str0++;
        for (;;) 
        {
          if (*str0 == '\n' || *str0 == '\0')
            error_exit("line %d: skip-field cannot be an incomplete string", line_id);
          else if (*str0 == '\\')
          {
            str0++;
            if (*str0 == '\"' || *str0 == '\\')
              str0++;
          }
          else if (*str0 == '\"')
          {
            str0++;
            break;
          }
          else
            str0++;
        }
      }
      else if ((*str0 >= '0' && *str0 <= '9') || *str0 == 'T' || *str0 == 't' || *str0 == 'F' || *str0 == 'f')
        while (*str0 != ' ' && *str0 != '\n' && *str0 != '\0')
          str0++;
      else
        error_exit("line %d: A skip-field has to be a valid datatype", line_id);

      break;
    }
  }

  return cnt_len;
}