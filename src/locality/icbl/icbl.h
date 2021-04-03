#ifndef ICBL
#define ICBL
#include "../../access/in_memory_file.h"
#include "../../data-struct/dict_ul.h"
#include "../../record/node.h"

#define MEMORY (8589934592.0f)

typedef struct dendrogram
{
    char*         label;
    unsigned long block_no;
    size_t        size;
    size_t        uncapt_s;

    union
    {
        unsigned long      node;
        struct dendrogram* dendro[2];
    } children;
} dendrogram_t;

size_t
get_num_steps(in_memory_file_t* db);

size_t
get_num_walks(in_memory_file_t* db);

float
weighted_jaccard_dist(dict_ul_ul_t* dif_set_a, dict_ul_ul_t* dif_set_b);

int
identify_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets);

int
cluster_coarse(in_memory_file_t* db,
               dict_ul_ul_t**    dif_sets,
               unsigned long*    parts);

int
block_formation(in_memory_file_t*    db,
                dict_ul_ul_t**       dif_sets,
                const unsigned long* parts,
                dendrogram_t***      blocks,
                unsigned long*       block_count,
                dendrogram_t****     block_roots);

int
layout_blocks(in_memory_file_t* db,
              dendrogram_t***   blocks,
              unsigned long*    block_count,
              unsigned long*    partitions,
              dendrogram_t***   block_roots);

unsigned long*
icbl(in_memory_file_t* db);

#endif
