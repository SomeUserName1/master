#ifndef ICBL
#define ICBL
#include "../../record/node.h"
#include "../../access/in_memory_file.h"
#include "../../data-struct/dict_ul.h"

#define MEMORY 8589934592.0f

typedef struct dendrogram {
    char* label;
    bool leaf[2];

    union {
        node_t* node;
        struct dendrogram* dendro;
    } children[2];
} dendrogram_t;

int gen_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets);
int cluster_coarse(in_memory_file_t* db, dict_ul_ul_t** dif_sets, size_t* parts);
int cluster_subgraphs(in_memory_file_t* db, dict_ul_ul_t** dif_sets, size_t* parts, dendrogram_t* root);
int create_blocks(in_memory_file_t* db, size_t* parts, dendrogram_t** roots);

#endif
