#ifndef __IN_MEMORY_FILE__
#define __IN_MEMORY_FILE__

#include "../data-types/dict_ul.h"
#include "../data-types/list_node.h"
#include "../data-types/list_rel.h"
#include "records/node.h"
#include "records/relationship.h"

typedef struct in_memory_file {
    dict_ul_node_t* cache_nodes;
    dict_ul_rel_t* cache_rels;
} in_memory_file_t;

in_memory_file_t* create_in_memory_file(void);
void in_memory_file_destroy(in_memory_file_t* db);

node_t* in_memory_get_node(in_memory_file_t* db, unsigned long id);
list_node_t* in_memory_get_nodes(in_memory_file_t* db);

relationship_t* in_memory_get_relationship(in_memory_file_t* db, unsigned long id);
list_relationship_t* in_memory_get_relationships(in_memory_file_t* db);

#endif 
