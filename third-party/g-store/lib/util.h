#ifndef UTIL_H
#define UTIL_H

#include "defs.h"

void error_exit(char* str0, ...);
void print_byte(one_byte* ob, int num);
void print_int(int i);
void print_arr(int* arr, int n);
void print_arr_uint2(uint2* arr, int n);
void print_endln();
void do_nothing();
void print_nl(char* str0, ...);
void print_ln(char* str0, ...);
void print_debug_ln(char* str0, ...);
void print_debug(char* str0, ...);
void free_all(int num, ...);
void* malloc_b(int n, char *msg, int mode = 0);               //TODO: Depreciate
void* realloc_b(void* &x, int n, char *msg, int mode = 0);    //TODO: Depreciate
int strcpystr (char* &str_source, char* str_dest);
void rnd_string(char* s, int len);
double rnd_normal(double mean, double stddev);
void permute(int *perm, int num);
void test_heap();
void strcat1(char* dest, char* a, char* b);
void change_p_lists(int v_id, int from_p, int to_p, int* v_per_p, int* v_per_p_begin);
void combine_p_lists(int first_p, int last_p, int new_p_num, int* v_per_p, int* v_per_p_begin);
void str_app_adv(char* &s, char c);
void str_app_adv(char* &s, char* t);
graph1* malloc_graph1_init(char *msg);
void free_graph1(graph1* graph);
int read_int();
bool confirm_yn(char* str, ...);
bool str_tok_cmp(char* &str0, char* tok, char mode = 0, char* str_error="");
bool str_tok_cmp_adv(char* &str0, char* tok, char mode = 0, char* str_error="");
bool str_tok_cmp_adv_qd(char* &str0, char* tok, char mode = 0);
bool str_tok_cmp_adv_sd(char* &str0, char* tok, char mode = 0);
bool str_tok_cmp_adv_menu(char* &str0, char* tok, char mode = 0);
bool str_tok_cmp_qd(char* &str0, char* tok, char mode = 0);
bool str_tok_cmp_sd(char* &str0, char* tok, char mode = 0);
bool str_tok_cmp_menu(char* &str0, char* tok, char mode = 0);
void str_eat_ws(char* &str0);
void str_eat_ws_err(char* &str0, char* str_error);
void str_eat_ws_qd(char* &str0);
void str_eat_ws_sd(char* &str0);
void str_eat_ws_tok_err(char* &str0, char tok, char* str_error);
void str_eat_ws_tok_qd(char* &str0, char tok);
void str_eat_ws_tok_sd(char* &str0, char tok);
void str_eat_ws_tok_menu(char* &str0, char tok);
bool str_is_ws_err(char* str0, char* str_error, char alt_sym1 = ' ', char alt_sym2 = ' ');
bool str_is_ws_qd(char* str0, char alt_sym1 = ' ', char alt_sym2 = ' ');
bool str_is_ws_sd(char* str0, char alt_sym1 = ' ', char alt_sym2 = ' ');
void write_param_arr(FILE* fp, char* str0, void* arr, int len);
void write_param_char(FILE* fp, char* str0, char* val);
bool read_param_char(FILE* fp, char* &str0, char* str1, char* val, int max_len);

bool strisnew(char* str_new, char* str_last);
char mystrncmp(char* str0, char* str1, int len);
void str_cpy_until_wsnl_adv(char* &str0_in, char* &str0_out);
int str_read_after_quote_adv(char* &str_in, char* &str_out);
void str_skip_adv(char* &str0);
int parse_field_id_adv(char* &str0);
LARGE_INTEGER to_li(int8 i);
int8 get_threshold(int c_level);
char* replace_if_reserved(char* str0, char* reserved_list[]);
char mytoupper(char c);

bool fkt_eq(char* a, char* b, int len);
bool fkt_neq(char* a, char* b, int len);
bool fkt_sm_eq(char* a, char* b, int len);
bool fkt_sm(char* a, char* b, int len);  
bool fkt_gr_eq(char* a, char* b, int len);
bool fkt_gr(char* a, char* b, int len);
void put_mult_char(char c, int num);
int len_num_str(int i);

#endif