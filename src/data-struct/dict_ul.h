#ifndef DICT_UL_H
#define DICT_UL_H

#include "../record/node.h"
#include "../record/relationship.h"
#include "htable.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct dict_ul_ul dict_ul_ul_t;
typedef struct dict_ul_int dict_ul_int_t;
typedef struct dict_ul_node dict_ul_node_t;
typedef struct dict_ul_rel dict_ul_rel_t;
typedef struct dict_ul_ul_iterator dict_ul_ul_iterator_t;
typedef struct dict_ul_int_iterator dict_ul_int_iterator_t;
typedef struct dict_ul_node_iterator dict_ul_node_iterator_t;
typedef struct dict_ul_rel_iterator dict_ul_rel_iterator_t;

dict_ul_ul_t*
create_dict_ul_ul(void);

int
dict_ul_ul_destroy(dict_ul_ul_t* ht);

size_t
dict_ul_ul_size(dict_ul_ul_t* ht);

int
dict_ul_ul_insert(dict_ul_ul_t* ht, unsigned long key, unsigned long value);

int
dict_ul_ul_remove(dict_ul_ul_t* ht, unsigned long key);

int
dict_ul_ul_get(dict_ul_ul_t* ht, unsigned long key, unsigned long** value);
unsigned long
dict_ul_ul_get_direct(dict_ul_ul_t* ht, unsigned long key);

bool
dict_ul_ul_contains(dict_ul_ul_t* ht, unsigned long key);

dict_ul_ul_iterator_t*
create_dict_ul_ul_iterator(dict_ul_ul_t* ht);

int
dict_ul_ul_iterator_next(dict_ul_ul_iterator_t* hi,
                         unsigned long** key,
                         unsigned long** value);

void
dict_ul_ul_iterator_destroy(dict_ul_ul_iterator_t* hi);

void
dict_ul_ul_print(dict_ul_ul_t* dict);

dict_ul_int_t*
create_dict_ul_int(void);

int
dict_ul_int_destroy(dict_ul_int_t* ht);

size_t
dict_ul_int_size(dict_ul_int_t* ht);

int
dict_ul_int_insert(dict_ul_int_t* ht, unsigned long key, int value);

int
dict_ul_int_remove(dict_ul_int_t* ht, unsigned long key);

int
dict_ul_int_get(dict_ul_int_t* ht, unsigned long key, int** value);

int
dict_ul_int_get_direct(dict_ul_int_t* ht, unsigned long key);

bool
dict_ul_int_contains(dict_ul_int_t* ht, unsigned long key);

dict_ul_int_iterator_t*
create_dict_ul_int_iterator(dict_ul_int_t* ht);

int
dict_ul_int_iterator_next(dict_ul_int_iterator_t* hi,
                          unsigned long** key,
                          int** value);

void
dict_ul_int_iterator_destroy(dict_ul_int_iterator_t* hi);

void
dict_ul_int_print(dict_ul_int_t* dict);

dict_ul_node_t*
create_dict_ul_node(void);

int
dict_ul_node_destroy(dict_ul_node_t* ht);

size_t
dict_ul_node_size(dict_ul_node_t* ht);

int
dict_ul_node_insert(dict_ul_node_t* ht, unsigned long key, node_t* value);

int
dict_ul_node_remove(dict_ul_node_t* ht, unsigned long key);

int
dict_ul_node_get(dict_ul_node_t* ht, unsigned long key, node_t** value);

node_t*
dict_ul_node_get_direct(dict_ul_node_t* ht, unsigned long key);

bool
dict_ul_node_contains(dict_ul_node_t* ht, unsigned long key);

dict_ul_node_iterator_t*
create_dict_ul_node_iterator(dict_ul_node_t* ht);

int
dict_ul_node_iterator_next(dict_ul_node_iterator_t* hi,
                           unsigned long** key,
                           node_t** value);

void
dict_ul_node_iterator_destroy(dict_ul_node_iterator_t* hi);

void
dict_ul_node_print(dict_ul_node_t* dict);

dict_ul_rel_t*
create_dict_ul_rel(void);

int
dict_ul_rel_destroy(dict_ul_rel_t* ht);

size_t
dict_ul_rel_size(dict_ul_rel_t* ht);

int
dict_ul_rel_insert(dict_ul_rel_t* ht, unsigned long key, relationship_t* value);

int
dict_ul_rel_remove(dict_ul_rel_t* ht, unsigned long key);

int
dict_ul_rel_get(dict_ul_rel_t* ht, unsigned long key, relationship_t** value);

relationship_t*
dict_ul_rel_get_direct(dict_ul_rel_t* ht, unsigned long key);

bool
dict_ul_rel_contains(dict_ul_rel_t* ht, unsigned long key);

dict_ul_rel_iterator_t*
create_dict_ul_rel_iterator(dict_ul_rel_t* ht);

int
dict_ul_rel_iterator_next(dict_ul_rel_iterator_t* hi,
                          unsigned long** key,
                          relationship_t** value);

void
dict_ul_rel_iterator_destroy(dict_ul_rel_iterator_t* hi);

void
dict_ul_rel_print(dict_ul_rel_t* dict);

#endif
