#include "util.h"

#include <conio.h>

#include "util_t.h"
#include "move_to_disk.h"
#include "structs.h"
#include "parameters.h"

/* Custom functions used in various algorithms */
//#include <g-store.h>

void error_exit(char* str0, ...)
{
  va_list vl;
  char* out1 = (char*) malloc(MAXLINE);
  if (out1 == NULL)
    printf("A memory error occurred. "
      "Not enough memory to display more information on the error\n\n");
  else
  {
    va_start(vl, str0);
    vsprintf(out1, str0, vl);
    va_end(vl);

    printf("Error! %s\n\n", out1);
  }

  if (*MY_NAME == '\0')
  {
    printf("Press any key to exit");
    _getch(); 
  }
  else
    WinExec(MY_NAME, SW_SHOW);

  exit(EXIT_FAILURE);
}


void print_byte(one_byte* ob, int num)
{
  for (int i=0; i<num; i++)
    printf("%d%d%d%d%d%d%d%d ",(ob+i)->_0,(ob+i)->_1,(ob+i)->_2,(ob+i)->_3,(ob+i)->_4,(ob+i)->_5,(ob+i)->_6,(ob+i)->_7);

  printf("\n");
};

void print_int(int i)
{
  printf("%d\n",i);
}

void print_arr(int* arr, int n)
{
  for (int i = 0; i < n; i++) 
    printf("%d ",arr[i]);

  printf("\n");
}

void print_arr_uint2(uint2* arr, int n)
{
  for (int i = 0; i < n; i++) 
    printf("%d ",arr[i]);

  printf("\n");
}

void print_endln()
{
  printf("\n");
}

void do_nothing()
{
  return;
}

void print_nl(char* str0, ...)
{
  va_list vl;
  char out1[256];

  va_start(vl, str0);
  vsprintf(out1, str0, vl);
  va_end(vl);

  printf("%s", out1);
}

void print_ln(char* str0, ...)
{
  va_list vl;
  char out1[256];

  va_start(vl, str0);
  vsprintf(out1, str0, vl);
  va_end(vl);

  printf("%s\n", out1);
}

void print_debug_ln(char* str0, ...)
{
  if (!PARAM::debug_mode)
    return;

  va_list vl;
  char out1[256];

  va_start(vl, str0);
  vsprintf(out1, str0, vl);
  va_end(vl);

  printf("%s\n", out1);
}

void print_debug(char* str0, ...)
{
  if (!PARAM::debug_mode)
    return;

  va_list vl;
  char out1[256];

  va_start(vl, str0);
  vsprintf(out1, str0, vl);
  va_end(vl);

  printf("%s", out1);
}

void free_all(int num, ...)
{
  void* ptr;
  va_list vl;

  va_start(vl, num);

  for (int i = 0; i<num; i++)
  {
    ptr = va_arg(vl,void*);
    free(ptr);

  }
}

//////////////////////////////////////////////////////////////////////////

//TO BE DEPRECIATED
void* malloc_b(int n, char *msg, int mode /*= 0*/)
{
  if (n == 0) 
    return NULL;

  void *ptr = malloc(n);

  if (ptr == NULL) 
    if (mode == 0)
      error_exit("cannot allocate %d bytes for %s", n, msg);
    else
      do
        if (move_graph_to_disk(mode) == -1)
          error_exit ("cannot allocate %d bytes for %s"
                      " - trying to move unused graph data to disk failed", n, msg);
      while ((ptr = malloc(n)) == NULL);

  return ptr;
}

void* realloc_b(void* &x, int n, char *msg, int mode /*= 0*/)
{
  if (n == 0)
  {
    free(x);
    return NULL;
  }

  void* ptr = realloc(x, n);

  if (ptr == NULL) 
    if (mode == 0)
      error_exit("cannot re-allocate %d bytes for %s", n, msg);
    else
      do
  if (move_graph_to_disk(mode) == -1)
    error_exit ("cannot re-allocate %d int for %s"
    " - trying to move unused graph data to disk failed", n, msg);
  while ((ptr = realloc(x, n)) == NULL);

  return ptr;
}



int strcpystr(char* &str_source, char* str_dest)
{
  while (*str_source++ != '\"')
    ;
  char *str_start = str_source;
  int i = 0;
  while (*str_source++ != '\"')
    i++;
  strncpy(str_dest,str_start,i);
  str_dest[i]='\0';
  return i;
}

//////////////////////////////////////////////////////////////////////////
// Used for generating a random graph
void rnd_string(char* s, int len) 
{
  char letters[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
    s[i] = letters[rand() % (sizeof(letters) - 1)];
  }

  s[len] = '\0';
}

//////////////////////////////////////////////////////////////////////////
// Used for generating a random graph
double rnd_normal(double mean, double stddev)
{
  static double n2 = 0.0;
  static int n2_cached = 0;
  if (!n2_cached) {
    double x, y, r;
    do {
      x = 2.0*rand()/RAND_MAX - 1;
      y = 2.0*rand()/RAND_MAX - 1;

      r = x*x + y*y;
    } while (r == 0.0 || r > 1.0);

    {
      double d = sqrt(-2.0*log(r)/r);
      double n1 = x*d;
      n2 = y*d;

      double result = n1*stddev + mean;

      n2_cached = 1;
      return result;
    }
  } else {
    n2_cached = 0;
    return n2*stddev + mean;
  }
}

//////////////////////////////////////////////////////////////////////////

void put_mult_char(char c, int num)
{
  for (int i = 0; i< num; i++)
    putchar(c);
}


//////////////////////////////////////////////////////////////////////////

//void permute(int *perm, int num)		// OLD, TBD!
//{
//  int tmp, rnd;
//
//  for (int i = 0; i < num; i++) {
//    rnd = rand()%num;
//    tmp = perm[i];
//    perm[i]=perm[rnd];
//    perm[rnd]=tmp;
//  }
//}

void permute(int *perm, int num)
{
  int tmp, rnd;

  for (int i = num-1; i > 0; i--) {
    rnd = rand()%(i+1);
    tmp = perm[i];
    perm[i]=perm[rnd];
    perm[rnd]=tmp;
  }
}

void test_heap()
{
  int* test_ptr = malloc1<int>(10000000, "test_heap");  //allocate 10M int
  assert(_CrtCheckMemory( ));
  free (test_ptr);
}


//////////////////////////////////////////////////////////////////////////

void strcat1(char* dest, char* a, char* b)
{
  while (*dest++ = *a++)
    ;
  dest--;
  while (*dest++ = *b++)
    ;
}

//////////////////////////////////////////////////////////////////////////

void change_p_lists(int v_id, int from_p, int to_p, int* v_per_p, int* v_per_p_begin)
{
  int i;

  if ((i = v_per_p_begin[from_p]) == v_id)
    v_per_p_begin[from_p] = v_per_p[i];
  else
  {
    while (v_per_p[i] != v_id)
      i = v_per_p[i];

    v_per_p[i] = v_per_p[v_id];
  }

  if ((i = v_per_p_begin[to_p]) == -1 || i > v_id)
  {
    v_per_p[v_id] = v_per_p_begin[to_p];
    v_per_p_begin[to_p] = v_id;
  }
  else
  {
    while (v_per_p[i] != -1 && v_per_p[i] < v_id)
      i = v_per_p[i];

    v_per_p[v_id] = v_per_p[i];
    v_per_p[i] = v_id;
  }
}

//////////////////////////////////////////////////////////////////////////

void combine_p_lists(int first_p, int last_p, int new_p_num, int* v_per_p, int* v_per_p_begin)
{
  //printf("%d %d %d\n", first_p, last_p, last_p - first_p + 1);
  assert(first_p <= last_p);

  int i, best_i = -1, prev_v = -1;
  int* next_arr_ = malloc1<int>(last_p - first_p + 2, "todo", 1);
  int* next_arr = next_arr_ + 1;
  next_arr[-1] = INT_MAX;

  for (i = first_p; i <= last_p; i++)
    if ((next_arr[i - first_p] = v_per_p_begin[i]) != -1 && v_per_p_begin[i] < next_arr[best_i])
      best_i = i - first_p;
  
  if (best_i != -1)
    v_per_p_begin[new_p_num] = next_arr[best_i];
  else
  {
    v_per_p_begin[new_p_num] = -1;
    free(next_arr_);
    return;
  }
  
  for(;;)
  {
    prev_v = next_arr[best_i];
    next_arr[best_i] = v_per_p[prev_v];
    best_i = -1;

    for (i = 0; i <= last_p - first_p; i++)
      if (next_arr[i] != -1 && next_arr[i] < next_arr[best_i])
        best_i = i;

    if (best_i == -1)
    {
      v_per_p[prev_v] = -1;
      break;
    }
    else
      v_per_p[prev_v] = next_arr[best_i];
  }
  free(next_arr_);
}

//////////////////////////////////////////////////////////////////////////

void str_app_adv(char* &s, char c)
{
  *s++ = c;
}

void str_app_adv(char* &s, char* t)
{
  while (*t != '\0')
    *s++ = *t++;
}


graph1* malloc_graph1_init(char *msg)
{
  graph1* ptr = (graph1*) malloc(sizeof(graph1));

  if (ptr == NULL) 
    error_exit("cannot allocate graph1 for %s", msg);

  ptr->init();

  return ptr;
}

void free_graph1(graph1* graph)
{
  free_all(8, graph->offset_e, graph->v_w, graph->map_in_c, graph->e_to, graph->e_w, graph->part, graph->p_w, graph);
}


//////////////////////////

/*uint read_uint()
{
  char line [MAXLINE];
  bool done;
  uint tmp;
  int i;
 
  i = 0;
  done = false;
  
  while (!done) 
  {
    line[i] = getchar();
    if (isdigit(line[i]))
      i++;
    else if (i != 0)
      done = true;
  }
  sscanf(line, "%u", &tmp);
  return tmp;
}*/

int read_int()
{
  char* line = malloc1<char>(MAXLINE, "todo");
  bool done = false;
  int tmp, i = 0;

  do
  {

    line[i] = getchar();
    if (isdigit(line[i]) || line[i] == '-')
      i++;
    else if (i != 0)
      done = true;
  } while (!done);

  sscanf(line, "%d", &tmp);
  free(line);
  return tmp;
}

//////////////////////////////////////////////////////////////////////////

bool confirm_yn(char* str, ...)
{

  va_list vl;
  char out1[256];

  va_start(vl, str);
  vsprintf(out1, str, vl);
  va_end(vl);

  char reply;
  bool result, done = false;
  void bail();

  printf("%s\n", out1);
  
  do
  {
    reply = char(getchar());
    
    while (reply == ' ' || reply == '\n') 
      reply = char(getchar());

    if (reply == 'y' || reply == 'Y')
    {
      result = true;
      done = true;
    }

    else if (reply == 'n' || reply == 'N')
    {
      result = false;
      done = true;
    }
    else 
    {
      printf("Valid responses begin with: y Y n N\n\n");
      printf("%s\n", out1);
      while (reply != '\n')       //Flush remaining characters
        reply = (char) getchar();
    }
  } while (!done);
  
  return result;
}


/////////////////////////////////////////////////////////////////////////// PARSING

//mode 1: take any non-letters
//mode 2: no requirements
//mode 3: require whitespace, produce error if no whitespace
//mode 4: require whitespace and token, produce error if either one not available
//any other: require whitespace
bool str_tok_cmp(char* &str0, char* tok, char mode /*= 0*/, char* str_error /* = ""*/)
{
  char* str1 = str0;
  char* tok_start = tok;

  while (*str1 == *tok)
  {
    str1++;
    tok++;
  }

  if (*str1 == '\0')
    if (*str_error == '\0')
      error_exit("Unexpected end of definition.");    
    else
      error_exit("Unexpected end of %s.", str_error);

  if (*tok == '\0' && (    mode == 2 
                       || (mode == 1 && !isalnum(*str1)) 
                       || (*str1 == ' ' || *str1 == '\t') ) )
  {
    return true;
  }
  else
  {
    if (mode == 3 || mode == 4)
      if (*tok == '\0')
        error_exit("Missing whitespace in %s.\nNot parsed: %s", str_error, str0);
      else if (mode == 4)
        error_exit("Expected '%s' in %s.\nNot parsed: %s", tok_start, str_error, str0);

    return false;
  }
}


// TODO check if \0 is really ok

//mode 1: take any non-letters
//mode 2: no requirements
//mode 3: require whitespace, produce error if no whitespace
//mode 4: require whitespace and token, produce error if either one not available

bool str_tok_cmp_adv(char* &str0, char* tok, char mode /*= 0*/, char* str_error /*= ""*/)
{
  char* str1 = str0;
  char* tok_start = tok;

  while (*str1 == *tok)
  {
    str1++;
    tok++;
  }

  if (*str1 == '\0')
    if (*str_error == '\0')
      error_exit("Unexpected end of definition.");    
    else
      error_exit("Unexpected end of %s.", str_error);

  if (*tok == '\0' && (    mode == 2 
                       || (mode == 1 && !isalnum(*str1)) 
                       || (*str1 == ' ' || *str1 == '\t') ) )
  {
    str0 = str1;
    return true;
  }
  else
  {
    if (mode == 3 || mode == 4)
      if (*tok == '\0')
        error_exit("Missing whitespace in %s.\nNot parsed: %s", str_error, str0);
      else if (mode == 4)
        error_exit("Expected '%s' in %s.\nNot parsed: %s", tok_start, str_error, str0);

    return false;
  }
}

bool str_tok_cmp_adv_qd(char* &str0, char* tok, char mode /*= 0*/)
{
  return str_tok_cmp_adv(str0, tok, mode, "query definition");
}

bool str_tok_cmp_adv_sd(char* &str0, char* tok, char mode /*= 0*/)
{
  return str_tok_cmp_adv(str0, tok, mode, "schema definition");
}

bool str_tok_cmp_adv_menu(char* &str0, char* tok, char mode /*= 0*/)
{
  return str_tok_cmp_adv(str0, tok, mode, "menu input");
}

bool str_tok_cmp_qd(char* &str0, char* tok, char mode /*= 0*/)
{
  return str_tok_cmp(str0, tok, mode, "query definition");
}

bool str_tok_cmp_sd(char* &str0, char* tok, char mode /*= 0*/)
{
  return str_tok_cmp(str0, tok, mode, "schema definition");
}

bool str_tok_cmp_menu(char* &str0, char* tok, char mode /*= 0*/)
{
  return str_tok_cmp(str0, tok, mode, "menu input");
}


//////////////////////////////////////////////////////////////////////////

void str_eat_ws(char* &str0)
{
  while (*str0 == ' ' || *str0 == '\t')
    str0++;
}

//////////////////////////////////////////////////////////////////////////

void str_eat_ws_err(char* &str0, char* str_error)
{
  if (*str0 != ' ' && *str0 != '\t')
    error_exit("missing whitespace in %s\nNot parsed: %s", str_error, str0);
  
  while (*str0 == ' ' || *str0 == '\t')
    str0++;
}

void str_eat_ws_qd(char* &str0)
{
  str_eat_ws_err(str0, "query definition");
}

void str_eat_ws_sd(char* &str0)
{
  str_eat_ws_err(str0, "schema definition");
}

//////////////////////////////////////////////////////////////////////////

void str_eat_ws_tok_err(char* &str0, char tok, char* str_error)
{
  str_eat_ws(str0);

  if (*str0 != tok)
    error_exit("expected character '%c' in %s, but found '%c'\nNot parsed: %s", 
    tok, str_error, *str0, str0);
  str0++;
}

void str_eat_ws_tok_qd(char* &str0, char tok)
{
  str_eat_ws_tok_err(str0, tok, "query definition");
}

void str_eat_ws_tok_sd(char* &str0, char tok)
{
  str_eat_ws_tok_err(str0, tok, "schema definition");
}

void str_eat_ws_tok_menu(char* &str0, char tok)
{
  str_eat_ws_tok_err(str0, tok, "menu input");
}

//////////////////////////////////////////////////////////////////////////

//mode 1: accept any non-letters
//any other: require whitespace
bool str_is_ws_err(char* str0, char* str_error, char alt_sym1 /*= ' '*/, char alt_sym2 /*= ' '*/)
{
  if (*str0 == '\0')
    error_exit("unexpected end of %s.", str_error);

  return (*str0 == ' ' || *str0 == '\t' || *str0 == alt_sym1 || *str0 == alt_sym2);
}

bool str_is_ws_qd(char* str0, char alt_sym1 /*= ' '*/, char alt_sym2 /*= ' '*/)
{
  return str_is_ws_err(str0, "query definition", alt_sym1, alt_sym2);
}

bool str_is_ws_sd(char* str0, char alt_sym1 /*= ' '*/, char alt_sym2 /*= ' '*/)
{
  return str_is_ws_err(str0, "schema definition", alt_sym1, alt_sym2);
}

//////////////////////////////////////////////////////////////////////////

// version for arrays
void write_param_arr(FILE* fp, char* str0, void* arr, int len) 
{
  char tmp[32];
  inttostr(tmp, len, true);
  fputs(str0, fp);
  fputc(' ', fp);
  fputs(tmp, fp);
  fputc(' ', fp);
  fputs("(array)", fp);
  fputc('\n', fp);
  fwrite(arr, len, 1, fp);
  fputc('\n', fp);
}

//////////////////////////////////////////////////////////////////////////


bool read_param_char(FILE* fp, char* &str0, char* str1, char* val, int max_len)
{
  char* str0_begin = str0, *val_begin = val;
  int cnt_len = 1;
  
  if (!str_tok_cmp_adv(str0, str1))
    return false;

  str_eat_ws(str0);

  if (*str0++ != '\"')
    error_exit("cannot parse line\n\n%s\nfrom parameter file - "
               "note that strings must be enclosed with \" ", str0_begin);
  for(;;)
  {
    if (cnt_len == max_len)
      error_exit("cannot parse line\n\n%s\nfrom parameter file - "
      "max len_str0 for this parameter is %d", str0_begin, max_len);
    
    if (*str0 == '\\')
    {
      str0++;
      if (*str0 == '\"' || *str0 == '\\')
        *val++ = *str0++;
      else
        *val++ = '\\';
      cnt_len++;
    }
    else if (*str0 == '\"')
      break;
    else if (*str0 == '\n' || *str0 == '\0')
      error_exit("cannot parse line\n\n%s\nfrom parameter file - "
      "note that strings must be enclosed with \" ", str0_begin);
    else
    {
      *val++ = *str0++;
      cnt_len++;
    }
  }

  *val = '\0';

  print_debug_ln("read '%s' in %s", val_begin, str1);

  do 
    fgets(str0, MAXLINE, fp);
  while ((*str0 == '#' || *str0 == '\n') && !feof(fp));
  return true;
}

void write_param_char(FILE* fp, char* str0, char* val)
{  
  fputs(str0, fp);
  fputc(' ', fp);
  fputc('\"', fp);
  fputs(val, fp);
  fputc('\"', fp);
  fputc('\n', fp);
}

bool strisnew(char* str_new, char* str_last)
{
  while (*str_new == *str_last)
  {
    str_new++;
    str_last++;
  }
  if (*str_new == ' ' || *str_new == '\t')
    return false;

  while (*str_new != ' ' && *str_new != '\t' && *str_new != '\0')
    *str_last++ = *str_new++;

  *str_last = '\0';

  return true;
}

//TODO: make more efficient for individual cases
// str0 = record, str1 = pattern
char mystrncmp(char* str0, char* str1, int len_str0)
{
  if (len_str0==0)
    if (str1 == '\0')
      return 0;
    else
      return -1;

  char* tmp = str0;
  while (*str0 == *str1 && len_str0 > 0 && *str1 != '\0')
  {
    str0++;
    str1++;
    len_str0--;
  }

  if (*str1 == '\0' && (len_str0 == 0 || *str0 == '\0'))
    return 0;
  else if (*str0 > *str1)
    return 1;
  else if (*str0 < *str1)
    return -1;
  
  assert (false);
  return 0;
}

void str_cpy_until_wsnl_adv(char* &str0_in, char* &str0_out)
{
  while (*str0_in != ' ' && *str0_in != '\t' && *str0_in != '\n')
    *str0_out++ = *str0_in++;
}

int str_read_after_quote_adv(char* &str_in, char* &str_out)
{
  int cnt_str_len = 0;

  for (;;) 
  {
    if (*str_in < 32 || *str_in > 126)
    {
      str_in++;
      *str_out++ = '_';
      cnt_str_len++;
    }
    else if (*str_in == '\\')
    {
      str_in++;
      if (*str_in == '\"' || *str_in == '\\')
        *str_out++ = *str_in++;
      else
        *str_out++ = '\\';

      cnt_str_len++;
    }
    else if (*str_in == '\"')
    {
      str_in++;
      break;
    }
    else
    {
      *str_out++ = *str_in++;
      cnt_str_len++;
    }
  }

  return cnt_str_len;
}

void str_skip_adv(char* &str0)
{
  if (*str0 == '\"')
  {
    str0++;
    for (;;) 
    {
      if (*str0 == '\\')
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
  else 
    while (*str0 != ' ' && *str0 != '\n' && *str0 != '\0')
      str0++;

}

int parse_field_id_adv(char* &str0)
{
  char tmp_str[64];
  int field_id;
  char* str1;
  int i, str_len;

  //  through field_name
  if (isdigit(*str0))
  {
    field_id = strtol(str0, &str1, 10);
    str0 = str1;
    if (field_id >= (GLOBALS::num_fields)) // Int + 2 = Ext + 1 = NUM_FIELDS
      error_exit("Invalid field number %d in query definition", field_id);
  } 

  //  through field_name
  else
  {
    str_len = 0;
    str1 = tmp_str;

    while (!str_is_ws_qd(str0, ',', ')') && !str_is_ws_qd(str0, '<', '=') && !str_is_ws_qd(str0, '!', '>'))
    {
      str_app_adv(str1, *str0++);
      if (++str_len == 31)
      {
        *str1 = '\0';
        error_exit("Field name too long in query definition: %s..", tmp_str);
      }
    }

    str_app_adv(str1, ' ');
    str_app_adv(str1, '\0');
    str1 = tmp_str;
    i = 0;
    field_id = -1;

    for (i = 0; i < GLOBALS::num_fields; i++)
      if (str_tok_cmp(str1, GLOBALS::rec_fields[i].field_name))
      {
        field_id = i;
        break;
      }

      if (field_id == -1)
        error_exit("Unknown field name in query definition: %s", tmp_str);

  }
  return field_id;
}

LARGE_INTEGER to_li(int8 i)
{
  LARGE_INTEGER tmp;
  tmp.QuadPart = i;
  return tmp;
}

int8 get_threshold(int c_level)
{
  return int8(GLOBALS::blk_writable) * int(pow((1/(1-GLOBALS::dbl_avg_c_ratio)), c_level+1));
  //return int8(BLOCK_SIZE) * pow((1/(1-AVG_C_RATIO)), c_level);
}

char my_to_upper(char c)
{
  if (c>= 97 && c<=122)
    return c-32;
  else
    return c;
}

char* replace_if_reserved(char* str0, char* reserved_list[])
{
  int i = 0;
  static char tmp[32];
  char* str1;
  char* str0_start = str0;

  while (*(str1 = reserved_list[i++])  != '0')
  {
    str0 = str0_start;
    while (my_to_upper(*str0) == my_to_upper(*str1))
    {
      if (*str0 == '\0' && *str1 == '\0')
      {
        str1 = tmp;
        str0 = str0_start;
        while (*str0 != '\0')
          *str1++ = *str0++;

        if (str0 - str0_start > 30)
        {
          *str1 = '\0';
          str1--;
          *str1 = '1';

        }
        else
        {
          *str1 = '1';
          str1++;
          *str1 = '\0';
        }
        return tmp;
      } 
      else if (*str0 == '\0' || *str1 == '\0')
        return str0_start;

      str0++;
      str1++;
    }    
  }

  return str0_start;
}

int len_num_str(int i)
{
  int j = 0, k = 1;
  
  if (i < 0)
    j++;
  i = std::abs(i);

  while (k <= i)
  {
    k*=10;
    j++;
  }

  return j;
}

bool fkt_eq(char* a, char* b, int len)      {return mystrncmp(a, b, len) == 0;}
bool fkt_neq(char* a, char* b, int len)     {return mystrncmp(a, b, len) != 0;}
bool fkt_sm_eq(char* a, char* b, int len)   {return mystrncmp(a, b, len) <= 0;}
bool fkt_sm(char* a, char* b, int len)      {return mystrncmp(a, b, len) <  0;}
bool fkt_gr_eq(char* a, char* b, int len)   {return mystrncmp(a, b, len) >= 0;}
bool fkt_gr(char* a, char* b, int len)      {return mystrncmp(a, b, len) >  0;}