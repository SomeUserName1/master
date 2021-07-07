#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include "node.h"
#include "page_cache.h"
#include "relationship.h"

typedef struct
{
    const char* file_name;
    page_cache* cache;
} heap_file;

heap_file*
heap_file_create(const char* file_name);

void
heap_file_destroy(heap_file* hf);

void
create_node(heap_file* hf, unsigned long node_id, char* label);

void
create_relationship(heap_file*    hf,
                    unsigned long rel_id,
                    unsigned long node_from,
                    unsigned long node_to,
                    double        weight,
                    char*         label);

node_t*
read_node(heap_file* hf, unsigned long node_id);

relationship_t*
read_relationship(heap_file* hf, unsigned long rel_id);

void
update_node(heap_file* hf, unsigned long target_node_id, node_t* node_to_write);

void
update_relationship(heap_file*      hf,
                    unsigned long   target_rel_id,
                    relationship_t* rel_to_write);

void
delete_node(heap_file* hf, unsigned long node_id);

void
delete_relationship(heap_file* hf, unsigned long rel_id);

#endif
