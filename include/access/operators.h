#ifndef OPERATORS_H
#define OPERATORS_H

#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"

#define BUFFER_SIZE (512)

typedef enum record_id_type
{
    NODE = 0,
    REL  = 1,
    ALL  = 2
} record_id_t;

typedef enum
{
    OUTGOING = 0,
    INCOMING = 1,
    BOTH     = 2
} direction_t;

typedef struct in_memory_file
{
    dict_ul_node* cache_nodes;
    dict_ul_rel*  cache_rels;
    unsigned long node_id_counter;
    unsigned long rel_id_counter;
} in_memory_file_t;

void
ids_to_blocks(const char* in_path, const char* out_path, record_id_t type);

void
blocks_to_pages(const char* in_path, const char* out_path, record_id_t type);

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

list_node*
in_memory_get_nodes(in_memory_file_t* db);

relationship_t*
in_memory_get_relationship(in_memory_file_t* db, unsigned long id);
list_relationship*
in_memory_get_relationships(in_memory_file_t* db);

unsigned long
in_memory_next_relationship(in_memory_file_t* db,
                            unsigned long     node_id,
                            relationship_t*   rel,
                            direction_t       direction);
list_relationship*
in_memory_expand(in_memory_file_t* db,
                 unsigned long     node_id,
                 direction_t       direction);
relationship_t*
in_memory_contains_relationship_from_to(in_memory_file_t* db,
                                        unsigned long     node_from,
                                        unsigned long     node_to,
                                        direction_t       direction);
#endif
