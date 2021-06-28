#ifndef IN_MEMORY_OPERATORS_H
#define IN_MEMORY_OPERATORS_H

#include "access/in_memory_file.h"
#include "access/node.h"
#include "access/relationship.h"

array_list_node*
in_memory_get_nodes(in_memory_file_t* db);

array_list_relationship*
in_memory_get_relationships(in_memory_file_t* db);

unsigned long
in_memory_next_relationship_id(in_memory_file_t* db,
                               unsigned long     node_id,
                               relationship_t*   rel,
                               direction_t       direction);
array_list_relationship*
in_memory_expand(in_memory_file_t* db,
                 unsigned long     node_id,
                 direction_t       direction);
relationship_t*
in_memory_contains_relationship_from_to(in_memory_file_t* db,
                                        unsigned long     node_from,
                                        unsigned long     node_to,
                                        direction_t       direction);
#endif
