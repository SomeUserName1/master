#include "util_t.h"

#include "util.h"
#include "defs.h"

/* Specialization functions for the template functions in util_t.h */
//#include <g-store.h

template <>
void write_param<double>(FILE* fp, char* str0, double val)
{
  char tmp[32];
  sprintf(tmp,"%f",val);
  fputs(str0, fp);
  fputc(' ', fp);
  fputs(tmp, fp);
  fputc('\n', fp);
}

template <>
void write_param<char>(FILE* fp, char* str0, char val)
{
  fputs(str0, fp);
  fputc(' ', fp);
  fputc('\'', fp);
  fputc(val, fp);
  fputc('\'', fp);
  fputc('\n', fp);
}

template <>
bool read_param<bool>(FILE* fp, char* &str0, char* str1, bool &val)
{
  char* str0_begin = str0;

  if (!str_tok_cmp_adv(str0, str1))
    return false;

  str_eat_ws(str0);

  if (*str0 == '0' || *str0 == 'F' || *str0 == 'f')
    val = false;
  else if (*str0 == '1' || *str0 == 'T' || *str0 == 't')
    val = true;
  else
    error_exit("cannot parse line\n\n%s\nfrom parameter file - "
    "expected a bool value to start with 0 1 T t F f", str0_begin);

  print_debug_ln("read '%s' in %s", (val ? "true" : "false"), str1);

  do 
  fgets(str0, MAXLINE, fp);
  while ((*str0 == '#' || *str0 == '\n') && !feof(fp));
  return true;
}

template <>
bool read_param<double>(FILE* fp, char* &str0, char* str1, double &val)
{
  if (!str_tok_cmp_adv(str0, str1))
    return false;

  str_eat_ws(str0);

  val = strtod(str0, NULL);

  print_debug_ln("read '%f' in %s", val, str1);
  
  do 
  fgets(str0, MAXLINE, fp);
  while ((*str0 == '#' || *str0 == '\n') && !feof(fp));
  return true;
}

template <>
bool read_param<char>(FILE* fp, char* &str0, char* str1, char &val)
{
  char* str0_begin = str0;

  if (!str_tok_cmp_adv(str0, str1))
    return false;

  str_eat_ws(str0);

  if (*str0++ != '\'')
    error_exit("cannot parse line\n\n%s\nfrom parameter file - "
    "note that characters must be enclosed with \' ", str0_begin);

  val = *str0++;

  if (*str0++ != '\'')
    error_exit("cannot parse line\n\n%s\nfrom parameter file - "
    "note that characters must be enclosed with \' ", str0_begin);

  print_debug_ln("read '%c' in %s", val, str1);

  do 
  fgets(str0, MAXLINE, fp);
  while ((*str0 == '#' || *str0 == '\n') && !feof(fp));
  return true;
}

template <>
void typetostr_adv<char*>(char* &str_dest, char* str_source, int len)
{
  //*str_dest++ = '\"';

  while (len > 0 && *str_source != '\0') 
  {
    *str_dest++ = *str_source++;
    len--;
  }

  str_source += len;
  //*str_dest++ = '\"';
}

template <>
void typetostr_adv<double>(char* &str_dest, char* str_source, int len)
{
  sprintf(str_dest,"%f", *(double*)str_source);
  str_dest--;
}

template <>
void typetostr_adv<bool>(char* &str_dest, char* str_source, int len)
{
  if (*str_source == 0)
  {
    *str_dest++ = 'f';
    *str_dest++ = 'a';
    *str_dest++ = 'l';
    *str_dest++ = 's';
    *str_dest++ = 'e';
  }
  else
  {
    *str_dest++ = 't';
    *str_dest++ = 'r';
    *str_dest++ = 'u';
    *str_dest++ = 'e';
  }
}