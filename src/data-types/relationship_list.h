#ifndef __RELATIONSHIP_LIST_H__
#define __RELATIONSHIP_LIST_H__

#include <stdbool.h>
#include <stddef.h>

#include "list.h"
#include "../storage/records/relationship.h"

typedef struct relationship_list relationship_list_t;

typedef enum { 
    REL_LIST_NONE = 0,
    REL_LIST_ID = 1 << 0,
    REL_LIST_ALL = 1 << 1
} rel_list_flags_t;

relationship_list_t* create_relationship_list(rel_list_flags_t flags);
#endif
