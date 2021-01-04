#ifndef __DICT_UL_H__
#define __DICT_UL_H__

#include "htable.h"

typedef struct dict_ul_ul dict_ul_ul_t;
typedef struct dict_ul_int dict_ul_int_t;
typedef struct dict_ul_ul_iterator dict_ul_ul_iterator_t;
typedef struct dict_ul_int_iterator dict_ul_int_iterator_t;

dict_ul_ul_t* create_dict_ul_ul(void);
int dict_ul_ul_destroy(dict_ul_ul_t* ht);
int dict_ul_ul_insert(dict_ul_ul_t* ht, unsigned long key, unsigned long value);
int dict_ul_ul_remove(dict_ul_ul_t* ht, unsigned long key);
int dict_ul_ul_get(dict_ul_ul_t* ht, unsigned long key, unsigned long* value);
unsigned long dict_ul_ul_get_direct(dict_ul_ul_t* ht, unsigned long key);
bool dict_ul_ul_contains(dict_ul_ul_t* ht, unsigned long key);
dict_ul_ul_iterator_t* create_dict_ul_ul_iterator(dict_ul_ul_t* ht);
int dict_ul_ul_iterator_next(dict_ul_ul_iterator_t* hi, unsigned long* key, unsigned long* value);
void dict_ul_ul_iterator_destroy(dict_ul_ul_iterator_t* hi);

dict_ul_int_t* create_dict_ul_int(void);
int dict_ul_int_destroy(dict_ul_int_t* ht);
int dict_ul_int_insert(dict_ul_int_t* ht, unsigned long key, int value);
int dict_ul_int_remove(dict_ul_int_t* ht, unsigned long key);
int dict_ul_int_get(dict_ul_int_t* ht, unsigned long key, int* value);
int dict_ul_int_get_direct(dict_ul_int_t* ht, unsigned long key);
bool dict_ul_int_contains(dict_ul_ul_t* ht, unsigned long key);
dict_ul_int_iterator_t* create_dict_ul_int_iterator(htable_t* ht);
int dict_ul_int_iterator_next(dict_ul_int_iterator_t* hi, unsigned long* key, int* value);
void dict_ul_int_iterator_destroy(dict_ul_int_iterator_t* hi);
#endif
