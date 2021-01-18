#include "ids_to_io.h"
#include "../record/relationship.h"
#include "../record/node.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned long id_to_page(unsigned long id, size_t page_size, record_id_t type) {
    size_t factor = type == REL ? sizeof(relationship_t) : sizeof(node_t);
    return id * factor / page_size;
}

io_stats_t* ids_to_io(const char* path, size_t page_size, size_t block_size, record_id_t type) {
    FILE* log_file = fopen(path, "r");
    if (log_file == NULL) {
        printf("ids_to_io: Can't open file with path %s", path);
        return NULL;
    }
    char ignore[128];
    char read_type[128];
    unsigned long id;
    memset(ignore, 0, 128);
    memset(read_type, 0, 128);

    unsigned long page_node;
    unsigned long page_rel;
    unsigned long prev_page_node = UNINITIALIZED_LONG;
    unsigned long prev_page_rel = UNINITIALIZED_LONG;
    unsigned long num_pages_loaded = 0;

   while(fscanf(log_file, "%s %s %lu", ignore, read_type, &id) == 3) {
       if ((type == NODE || type == ALL) && strncmp(read_type, "Node", 4) == 0) {
            if ((page_node = id_to_page(id, page_size, NODE)) != prev_page_node) {
                prev_page_rel = page_node;
                num_pages_loaded++;
            }
       }
       if ((type == REL || type == ALL) && strncmp(read_type, "Relationship", 12) == 0) {
            if ((page_rel = id_to_page(id, page_size, REL)) != prev_page_rel) {
                prev_page_rel = page_rel;
                num_pages_loaded++;
            }
       }
    }
    io_stats_t* result = malloc(sizeof(*result));
    result->read_pages = num_pages_loaded;
    result->read_blocks = num_pages_loaded * page_size / block_size;
    result->write_pages = 0;
    result->write_blocks = 0;

    return result;
}

