#include "list_ul.h"
#include "cbs.h"
#include "list.h"

#include <stdlib.h>

list_ul_t* create_list_ul() {
    list_cbs_t cbs = {
        unsigned_long_eq,
        unsigned_long_copy,
        free
    };
    list_t* lst =  create_list(&cbs);

    return (list_ul_t*) lst;
}

void list_ul_destroy(list_ul_t* l) {
    return list_destroy((list_t*) l);
}

size_t list_ul_size(list_ul_t* l) {
    return list_size((list_t*) l);
}

int list_ul_append(list_ul_t* l, unsigned long v) {
    return list_append((list_t*) l, (void*) &v);
}

int list_ul_insert(list_ul_t* l, unsigned long v, size_t idx) {
    return list_insert((list_t*) l, (void*) &v, idx);
}

int list_ul_remove(list_ul_t* l, size_t idx) {
    return list_remove((list_t*) l, idx);
}

int list_ul_remove_elem(list_ul_t* l, unsigned long elem) {
    return  list_remove_elem((list_t*) l, (void*) &elem);
}

int list_ul_index_of(list_ul_t* l, unsigned long v, size_t* idx) {
    return list_index_of((list_t*) l, (void*) &v, idx);
}

bool list_ul_contains(list_ul_t* l, unsigned long v) {
    return list_contains((list_t*) l, (void*) &v);
}

unsigned long list_ul_get(list_ul_t* l, size_t idx) {
    return *((unsigned long*) list_get((list_t*) l, idx));
}

unsigned long list_ul_take(list_ul_t* l, size_t idx) {
    return *((unsigned long*) list_take((list_t*) l, idx));
}

