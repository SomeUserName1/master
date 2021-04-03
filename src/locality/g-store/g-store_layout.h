#ifndef ML_H
#define ML_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define ALPHA                     (0.125F)
#define BETA                      (1)
#define GAMMA                     (8)
#define REFINEMENT_ITERS          (1)
#define C_RATIO_LIMIT             (0.3F)
#define MAX_PARTITION_SIZE_FACTOR (32)

#include "../../access/in_memory_file.h"
#include "../../data-struct/list_ul.h"

typedef struct multi_level_graph
{
    unsigned int              c_level;
    in_memory_file_t*         records;
    unsigned long*            node_aggregation_weight;
    unsigned long*            edge_aggregation_weight;
    unsigned long*            map_to_coarser;
    unsigned long*            partition;
    unsigned long*            partition_aggregation_weight;
    unsigned long             num_partitions;
    struct multi_level_graph* finer;
    struct multi_level_graph* coarser;
} multi_level_graph_t;

int
coarsen(multi_level_graph_t* graph,
        size_t               block_size,
        size_t*              num_v_matches,
        size_t*              max_partition_size,
        float*               c_ratio_avg);

void
turn_around(multi_level_graph_t* graph, size_t block_size);

void
project(multi_level_graph_t* graph,
        bool*                part_type,
        size_t               block_size,
        float                c_ratio_avg,
        list_ul_t**          nodes_per_part);

void
reorder(multi_level_graph_t* graph, const bool* part_type);

void
refine(multi_level_graph_t* graph, size_t block_size, float c_ratio_avg);

int
uncoarsen(multi_level_graph_t* graph, size_t block_size, float c_ratio_avg);

void
finalize(multi_level_graph_t* db);

unsigned long*
g_store_layout(in_memory_file_t* db, size_t block_size);

#endif
