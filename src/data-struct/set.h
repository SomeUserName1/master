#ifndef SET
#define SET
#include "htable.h"

#include <stdbool.h>

typedef struct set set_t;

set_t*
create_set(htable_hash hash, htable_keq keq, htable_cbs_t* cbs);
void
set_destroy(set_t* set);
size_t
set_size(set_t* set);

int
set_insert(set_t* set, void* elem);
int
set_remove(set_t* set, void* elem);
bool
set_contains(set_t* set, void* elem);

#endif
