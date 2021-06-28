#ifndef HEAP_FILE_OPERATORS_H
#define HEAP_FILE_OPERATORS_H

#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"

array_list_node*
get_nodes(heap_file* hf);

array_list_relationship*
get_relationships(heap_file* hf);

unsigned long
next_relationship_id(heap_file*      hf,
                     unsigned long   node_id,
                     relationship_t* rel,
                     direction_t     direction);

array_list_relationship*
expand(heap_file* db, unsigned long node_id, direction_t direction);

relationship_t*
contains_relationship_from_to(heap_file*    hf,
                              unsigned long node_from,
                              unsigned long node_to,
                              direction_t   direction);
#endif
