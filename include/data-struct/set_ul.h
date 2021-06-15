#ifndef SET_UL
#define SET_UL

#include <stdbool.h>
#include <stddef.h>

typedef struct set_ul set_ul_t;

set_ul_t*
create_set_ul(void);

void
set_ul_destroy(set_ul_t* set);

size_t
set_ul_size(set_ul_t* set);

int
set_ul_insert(set_ul_t* set, unsigned long elem);

int
set_ul_remove(set_ul_t* set, unsigned long elem);

bool
set_ul_contains(set_ul_t* set, unsigned long elem);

#endif
