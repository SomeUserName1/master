#include "ids_to_io.h"
#include "../constants.h"
#include "../record/node.h"
#include "../record/relationship.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long
id_to_page(unsigned long id, size_t page_size, record_id_t type)
{
    size_t factor = type == REL ? sizeof(relationship_t) : sizeof(node_t);
    return id * factor / page_size;
}

io_stats_t*
ids_to_io(const char* in_path,
          const char* out_path,
          size_t page_size,
          size_t block_size,
          record_id_t type)
{
    if(!in_path || !out_path) {
        exit(-1);
    }

    FILE* log_file = fopen(in_path, "r");
    if (log_file == NULL) {
        printf("ids_to_io: Can't open file with path %s", in_path);
        return NULL;
    }
    FILE* page_accesses_file = fopen(out_path, "w");
    if (page_accesses_file == NULL) {
        fclose(log_file);
        printf("ids_to_io: Can't open file with path %s", out_path);
        return NULL;
    }
    char ignore[BUFFER_SIZE];
    char read_type[BUFFER_SIZE];
    unsigned long id;
    memset(ignore, 0, BUFFER_SIZE);
    memset(read_type, 0, BUFFER_SIZE);

    unsigned long page_node;
    unsigned long page_rel;
    unsigned long prev_page_node = UNINITIALIZED_LONG;
    unsigned long prev_page_rel = UNINITIALIZED_LONG;
    unsigned long num_pages_loaded = 0;
    size_t pages_to_blocks =
          (page_size / block_size) > 0 ? page_size / block_size : 1;

    while (fscanf(log_file, "%s %s %lu", ignore, read_type, &id) == 3) {
        if ((type == NODE || type == ALL) &&
            strncmp(read_type, "Node", 4) == 0) {
            if ((page_node = id_to_page(id, page_size, NODE)) !=
                prev_page_node) {
                prev_page_rel = page_node;
                num_pages_loaded++;
            }

            fprintf(page_accesses_file,
                    "%s%lu%s",
                    "Node: PageOffset: ",
                    page_node,
                    " BlockOffset:");
            for (size_t i = 0; i < pages_to_blocks; ++i) {
                fprintf(page_accesses_file,
                        " %lu",
                        page_node * pages_to_blocks + i);
            }
            fprintf(page_accesses_file, "\n");
        }
        if ((type == REL || type == ALL) &&
            strncmp(read_type, "Relationship", NUM_CHARS_REL) == 0) {
            if ((page_rel = id_to_page(id, page_size, REL)) != prev_page_rel) {
                prev_page_rel = page_rel;
                num_pages_loaded++;
            }

            fprintf(page_accesses_file,
                    "%s%lu%s",
                    "Relationship: PageOffset: ",
                    page_rel,
                    " BlockOffset:");
            for (size_t i = 0; i < pages_to_blocks; ++i) {
                fprintf(page_accesses_file,
                        " %lu",
                        page_rel * pages_to_blocks + i);
            }
            fprintf(page_accesses_file, "\n");
        }
    }
    io_stats_t* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->read_pages = num_pages_loaded;
    result->read_blocks = num_pages_loaded * page_size / block_size;
    result->write_pages = 0;
    result->write_blocks = 0;

    fflush(page_accesses_file);
    fclose(log_file);
    fclose(page_accesses_file);

    return result;
}
