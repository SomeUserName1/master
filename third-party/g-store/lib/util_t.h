#ifndef UTIL_T_H
#define UTIL_T_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "util.h"
#include "move_to_disk.h"

/* Custom functions used in various algorithms. See also util_s.cpp */

template <typename T_>
T_* malloc1(int n, char *msg, int mode = 0)
{
  if (n == 0)
    return NULL;

  void *ptr = malloc(n * sizeof(T_));

  if (ptr == NULL) 
    if (mode == 0)
      error_exit("cannot allocate %d bytes for %s", n * sizeof(T_), msg);
    else
      do
        if (move_graph_to_disk(mode) == -1)
          error_exit ("cannot allocate %d bytes for %s"
          " - trying to move unused graph data to disk failed", n * sizeof(T_), msg);
      while ((ptr = malloc(n * sizeof(T_))) == NULL);

  return (T_*) ptr;
}

template <typename T_>
T_* realloc1(T_* &x, int n, char *msg, int mode = 0)
{
  if (n == 0)
  {
    free(x);
    return NULL;
  }

  void* ptr = realloc(x, n * sizeof(T_));

  if (ptr == NULL) 
    if (mode == 0)
      error_exit("cannot re-allocate %d bytes for %s", n * sizeof(T_), msg);
    else
      do
  if (move_graph_to_disk(mode) == -1)
    error_exit ("cannot re-allocate %d int for %s"
    " - trying to move unused graph data to disk failed", n * sizeof(T_), msg);
  while ((ptr = realloc(x, n * sizeof(T_))) == NULL);

  return (T_*) ptr;
}

template <typename T_>
T_* arr_set(T_* x, T_ val, int n)
{
  for (int i = 0; i < n; i++) 
    x[i] = val;

  return x;
}

template <typename T_>
T_* malloc1_set(int n, T_ val, char *msg, int mode = 0)
{
  T_* ptr = malloc1<T_>(n, msg, mode);

  if (ptr == NULL) 
    return NULL;

  return arr_set<T_>(ptr, val, n);
};

template <typename T_>
void swap1(T_ &a, T_ &b)
{
  T_ tmp = a;
  a = b;
  b = tmp;
}

template <typename T_>
int abs1(T_ a)
{
  if (a < 0) 
    return -a;
  return a;
}


template <typename T_>
T_ min1(T_ a, T_ b)
{
  return (a < b ? a : b);
}

template <typename T_>
T_ max1(T_ a, T_ b)
{
  return (a > b ? a : b);
}

template <typename T_>
int bs_comp_t (const void* a, const void* b)
{
  if (*(T_*) a > *(T_*) b) 
    return 1;
  else if (*(T_*) a == *(T_*) b) 
    return 0;
  else 
    return -1;

}

template <typename T_>
int bs_comp_int (const void* a, const void* b)
{
  return *(T_*) a - *(T_*) b;
}


template <typename T_>
void if_gr_set(T_ &i, T_ &max_)
{
  if (i > max_)
    max_ = i;
}

template <typename T_>
void if_sm_set(T_ &i, T_ &min_)
{
  if (i < min_)
    min_ = i;
}

template <typename _T>
void write_param(FILE* fp, char* str0, _T val)
{
  char tmp[32];
  inttostr(tmp, val, true);
  fputs(str0, fp);
  fputc(' ', fp);
  fputs(tmp, fp);
  fputc('\n', fp);
}

template <> void write_param<double>(FILE* fp, char* str0, double val);
template <> void write_param<char>(FILE* fp, char* str0, char val);

template <typename _T>
bool read_param(FILE* fp, char* &str0, char* str1, _T &val)
{
  if (!str_tok_cmp_adv(str0, str1))
    return false;

  str_eat_ws(str0);

  val = (_T) _strtoi64(str0, NULL, 10);

  print_debug_ln("read '%lld' in %s", int8(val), str1);

  do 
  fgets(str0, MAXLINE, fp);
  while ((*str0 == '#' || *str0 == '\n') && !feof(fp));
  return true;
}

template <> bool read_param<bool>(FILE* fp, char* &str0, char* str1, bool &val);
template <> bool read_param<double>(FILE* fp, char* &str0, char* str1, double &val);
template <> bool read_param<char>(FILE* fp, char* &str0, char* str1, char &val);

template <typename _T>
bool read_param_arr(FILE* fp, char* &str0, char* str1, _T &arr)
{
  if (!str_tok_cmp_adv(str0, str1))
    return false;

  str_eat_ws(str0);

  int len = strtol(str0, NULL, 10);
  arr = (_T) malloc_b(len, "todo");
  fread(arr, len, 1, fp);
  fgetc(fp);              //newline character

  print_debug_ln("read array %s", str1);

  do 
  fgets(str0, MAXLINE, fp);
  while ((*str0 == '#' || *str0 == '\n') && !feof(fp));
  return true;
}

//////////////////////////////////////////////////////////////////////////

template <typename T_>
void inttostr(char* s, T_ i, bool end_str = false)
{
  assert(-1 % 2 < 0);

  char *str0 = s, *str1 = s;
  char tmp;

  if (i >= 0)
  {
    do
    *str1++ = '0' + (i % 10);
    while (i /= 10);
  }
  else
  {
    *str1++ = '-';
    str0++;

    do
    *str1++ = '0' - i % 10;
    while (i /= 10);
  }

  if (end_str)
    *(s + (str1 - str0)) = '\0';

  for (str1--; str0 < str1; str0++, str1--)
  {
    tmp = *str0;
    *str0 = *str1;
    *str1 = tmp;
  }
}

template <typename T_>
void inttostr_adv(char* &str_dest, T_ i)
{
  assert(-1 % 2 < 0);

  char *str0 = str_dest, *str1 = str_dest;
  char tmp;

  if (i >= 0)
  {
    do
    *str1++ = '0' + (i % 10);
    while (i /= 10);
  }
  else
  {
    *str1++ = '-';
    str0++;

    do
    *str1++ = '0' - i % 10;
    while (i /= 10);
  }

  str_dest += str1 - str0;

  for (str1--; str0 < str1; str0++, str1--)
  {
    tmp = *str0;
    *str0 = *str1;
    *str1 = tmp;
  }
}


template <typename T_>
void typetostr_adv(char* &str_dest, char* str_source, int len = -1)
{
  T_ i = *((T_*) str_source);
  inttostr_adv(str_dest, i);
}

template <> void typetostr_adv<char*>(char* &str_dest, char* str_source, int len);
template <> void typetostr_adv<double>(char* &str_dest, char* str_source, int len);
template <> void typetostr_adv<bool>(char* &str_dest, char* str_source, int len);


//////////////////////////////////////////////////////////////////////////

template <typename T_>
int get_lower_idx_1(T_ key, T_* x, int num_elem)
{
  return 0;
}

template <typename T_>
int get_lower_idx_n(T_ key, T_* x, int num_elem)
{
  T_* low = x, *high = x + num_elem, *center = x + num_elem/2;

  do
  {
    if (key == *center)
      return center - x;

    if (key > *center)
      low = center;
    else 
      high = center;

    center = low + (high - low)/2;
  } while (low + 1 != high);

  return low - x;
}

template <typename T_> bool fkt_eq(T_ val, T_ targ)      {return val == targ;};
template <typename T_> bool fkt_neq(T_ val, T_ targ)     {return val != targ;};
template <typename T_> bool fkt_sm_eq(T_ val, T_ targ)   {return val <= targ;};
template <typename T_> bool fkt_sm(T_ val, T_ targ)      {return val < targ;};
template <typename T_> bool fkt_gr_eq(T_ val, T_ targ)   {return val >= targ;};
template <typename T_> bool fkt_gr(T_ val, T_ targ)      {return val > targ;};

#endif