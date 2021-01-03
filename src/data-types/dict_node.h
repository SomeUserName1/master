#ifndef __NODE_REL_DICT_H__
#define __NODE_REL_DICT_H__

#include "htable.h"

typedef struct dict_node_rel dict_node_rel_t;
typedef struct dict_node_int dict_node_int_t;

dict_node_rel_t* create_dict_node_rel(void);
dict_node_int_t* create_dict_node_int(void);
#endif
