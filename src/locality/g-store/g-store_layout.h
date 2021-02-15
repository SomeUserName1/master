#ifndef ML_H
#define ML_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define ALPHA 0.125
#define BETA 1
#define GAMMA 8
#define REFINEMENT_ITERS 30

#include "../../access/in_memory_file.h"
#include "../../data-struct/list_ul.h"

typedef struct multi_level_graph {
    size_t c_level;
    in_memory_file_t* records;
    size_t* node_aggregation_weight;
    size_t* edge_aggregation_weight;
    size_t* map_to_coarser;
    size_t* partition;
    size_t* partition_aggregation_weight;
    size_t num_partitions;
    struct multi_level_graph* finer;
    struct multi_level_graph* coarser;
} multi_level_graph_t;

int coarsen(multi_level_graph_t* graph, size_t block_size, size_t* num_v_matches, size_t* max_partition_size, float* c_ratio_avg);
void turn_around(multi_level_graph_t* graph, size_t block_size);
void project(multi_level_graph_t* graph, bool* part_type, size_t block_size, float c_ratio_avg, list_ul_t** nodes_per_part);
void reorder(multi_level_graph_t* graph, bool* part_type);
void refine(multi_level_graph_t* db, size_t block_size, float c_ratio_avg);
int uncoarsen(multi_level_graph_t* db, size_t block_size, float c_ratio_avg);
void finalize(multi_level_graph_t* db);
void g_store_layout(in_memory_file_t* db, size_t block_size);

#endif
