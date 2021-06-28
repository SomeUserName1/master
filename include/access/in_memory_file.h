#ifndef IN_MEMORY_FILE_H
#define IN_MEMORY_FILE_H

#include "access/node.h"
#include "access/relationship.h"

typedef struct in_memory_file
{
    dict_ul_node* cache_nodes;
    dict_ul_rel*  cache_rels;
    unsigned long node_id_counter;
    unsigned long rel_id_counter;
} in_memory_file_t;

in_memory_file_t*
create_in_memory_file(void);

void
in_memory_file_destroy(in_memory_file_t* db);

unsigned long
in_memory_create_node(in_memory_file_t* db);

unsigned long
in_memory_create_relationship(in_memory_file_t* db,
                              unsigned long     node_from,
                              unsigned long     node_to);
unsigned long
in_memory_create_relationship_weighted(in_memory_file_t* db,
                                       unsigned long     node_from,
                                       unsigned long     node_to,
                                       double            weight);

node_t*
in_memory_get_node(in_memory_file_t* db, unsigned long id);

relationship_t*
in_memory_get_relationship(in_memory_file_t* db, unsigned long id);

#endif
