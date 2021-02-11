#ifndef ML_H
#define ML_H

#include "../../access/in_memory_file.h"
#include "../../data-struct/list_ul.h"

typedef struct multi_level_graph {
    in_memory_file_t* records;
    size_t* node_aggregation_weight;
    size_t* edge_aggregation_weight;
    size_t* map_to_coarser;
    struct multi_level_graph* finer;
    struct multi_level_graph* coarser;
    size_t c_level;
} multi_level_graph_t;

// ml_coarsen.cpp
int coarsen(multi_level_graph_t* graph, size_t block_size, size_t* num_v_matches, size_t* max_partition_size);
void coarsen_two(multi_level_graph_t* graph, int max_part_w, double* c_ratio);
void coarsen_n(multi_level_graph_t* graph, int num_v_matched, double* c_ratio);
void create_c_graph(multi_level_graph_t* graph, int new_v, int* v_per_p, int* v_per_p_begin);

// ml_part.cpp
void g_store_layout(in_memory_file_t* db, size_t block_size);

// ml_project.cpp
void project(multi_level_graph_t* db, int* v_per_p, int* v_per_p_begin, list_ul_t* part_type, int* max_p1);

// ml_turn_around.cpp
void turn_around(multi_level_graph_t* db);

// ml_uncoarsen.cpp
int uncoarsen(multi_level_graph_t* db);

// ml_reorder.cpp
void reorder(multi_level_graph_t* db, int* v_per_p, int* v_per_p_begin,  list_ul_t* part_type, int* max_p1);

// ml_refine.cpp
void refine(multi_level_graph_t* db, int* v_per_p, int* v_per_p_begin);

// finalize.cpp
void finalize(multi_level_graph_t* db, int* v_per_p, int* v_per_p_begin);

#endif
