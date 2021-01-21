#ifndef IN_MEMORY_FILE
#define IN_MEMORY_FILE

#include "../data-struct/dict_ul.h"
#include "../data-struct/list_node.h"
#include "../data-struct/list_rel.h"
#include "../record/node.h"
#include "../record/relationship.h"

typedef enum {
    OUTGOING = 0,
    INCOMING = 1,
    BOTH     = 2
} direction_t;

typedef struct in_memory_file {
    dict_ul_node_t* cache_nodes;
    dict_ul_rel_t* cache_rels;
    unsigned long rel_id_counter;
} in_memory_file_t;

in_memory_file_t* create_in_memory_file(void);
void in_memory_file_destroy(in_memory_file_t* db);

int in_memory_create_node(in_memory_file_t* db, unsigned long id);
int in_memory_create_relationship(in_memory_file_t* db, unsigned long nodeFrom, unsigned long nodeTo);
int in_memory_create_relationship_weighted(in_memory_file_t* db, unsigned long nodeFrom, unsigned long nodeTo, long double weight);

node_t* in_memory_get_node(in_memory_file_t* db, unsigned long id);
list_node_t* in_memory_get_nodes(in_memory_file_t* db);

relationship_t* in_memory_get_relationship(in_memory_file_t* db, unsigned long id);
list_relationship_t* in_memory_get_relationships(in_memory_file_t* db);

unsigned long in_memory_next_relationship(in_memory_file_t* db, unsigned long node_id, relationship_t* rel, direction_t direction);
list_relationship_t* in_memory_expand(in_memory_file_t* db, unsigned long node_id, direction_t direction);

#endif
