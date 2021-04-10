#include "ids_to_io.h"
#include "../constants.h"
#include "../record/node.h"
#include "../record/relationship.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
ids_to_blocks(const char* in_path, const char* out_path, record_id_t type)
{
    if (!in_path || !out_path) {
        exit(-1);
    }

    FILE* log_file = fopen(in_path, "r");
    if (log_file == NULL) {
        printf("ids_to_blocks: Can't open file with path %s", in_path);
    }
    FILE* block_file = fopen(out_path, "w");
    if (block_file == NULL) {
        fclose(log_file);
        printf("ids_to_blocks: Can't open file with path %s", out_path);
    }
    char          read_type[BUFFER_SIZE];
    unsigned long id;
    memset(read_type, 0, BUFFER_SIZE);

    while (fscanf(log_file, "%1s %lu\n", read_type, &id) == 2) {
        if (type != REL && strncmp(read_type, "N", 1) == 0) {
            fprintf(block_file,
                    "%s %lu\n",
                    "N",
                    (unsigned long)ceil((float)id * sizeof(node_t)
                                        / (float)BLOCK_SIZE));
        }
        if (type != NODE && strncmp(read_type, "R", 1) == 0) {
            fprintf(block_file,
                    "%s %lu\n",
                    "R",
                    (unsigned long)ceil((float)id * sizeof(relationship_t)
                                        / (float)BLOCK_SIZE));
        }
    }

    fflush(block_file);
    fclose(log_file);
    fclose(block_file);
}

void
blocks_to_pages(const char* in_path, const char* out_path, record_id_t type)
{
    if (!in_path || !out_path || BLOCK_SIZE == 0) {
        exit(-1);
    }

    float factor = floor((float)PAGE_SIZE / (float)BLOCK_SIZE);

    FILE* block_file = fopen(in_path, "r");
    if (block_file == NULL) {
        printf("blocks_to_pages: Can't open file with path %s", in_path);
    }
    FILE* page_file = fopen(out_path, "w");
    if (page_file == NULL) {
        fclose(block_file);
        printf("ids_to_blocks: Can't open file with path %s", out_path);
    }
    char          read_type[BUFFER_SIZE];
    unsigned long block;
    memset(read_type, 0, BUFFER_SIZE);

    while (fscanf(block_file, "%1s %lu\n", read_type, &block) == 2) {
        if ((type == NODE || type == ALL) && strncmp(read_type, "N", 1) == 0) {
            fprintf(page_file,
                    "%s %lu\n",
                    "N",
                    (unsigned long)ceil((float)block / (float)factor));
        }
        if (type != NODE && strncmp(read_type, "R", 1) == 0) {
            fprintf(page_file,
                    "%s %lu\n",
                    "R",
                    (unsigned long)ceil((float)block / (float)factor));
        }
    }

    fflush(page_file);
    fclose(page_file);
    fclose(block_file);
}
