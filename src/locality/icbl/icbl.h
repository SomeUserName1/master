#ifndef ICBL
#define ICBL
#include "../../record/node.h"
#include "../../access/in_memory_file.h"
#include "../../data-struct/dict_ul.h"

#define MEMORY (8589934592.0f)

typedef struct dendrogram {
    char* label;
    unsigned long block_no;
    size_t size;
    size_t uncapt_s;

    union {
        unsigned long node;
        struct dendrogram* dendro[2];
    } children;
} dendrogram_t;

int identify_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets);
int cluster_coarse(in_memory_file_t* db, dict_ul_ul_t** dif_sets,
        unsigned long* parts);
int block_formation(in_memory_file_t* db, dict_ul_ul_t** dif_sets,
        const unsigned long* parts, dendrogram_t*** blocks,
        unsigned long* block_count);
int layout_blocks(in_memory_file_t* db, dendrogram_t*** blocks,
        unsigned long* block_count);
int icbl(in_memory_file_t* db);

#endif
