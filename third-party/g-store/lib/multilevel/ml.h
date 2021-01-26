#ifndef ML_H
#define ML_H

#include "defs.h"
#include "structs.h"

// ml_coarsen.cpp
int coarsen(graph1 *graph);
void coarsen_two(graph1 *graph, int max_part_w, double &c_ratio);
void coarsen_n(graph1* graph, int num_v_matched, double &c_ratio);
void create_c_graph(graph1* graph, int new_v, int* v_per_p, int* v_per_p_begin);

// ml_part.cpp
void part_graph1(graph1 *graph);

// ml_project.cpp
void project(graph1* graph, int* v_per_p, int* v_per_p_begin, std::vector<bool>* part_type, int &max_p1);

// ml_turn_around.cpp
void turn_around(graph1 *graph);

// ml_uncoarsen.cpp
int uncoarsen(graph1 *graph, double &t1, double &t2, double &t3, double &t4);

// ml_reorder.cpp
void reorder(graph1* graph, int* v_per_p, int* v_per_p_begin, std::vector<bool>* part_type, int &max_p1);

// ml_refine.cpp
void refine(graph1* graph, int* v_per_p, int* v_per_p_begin);

// finalize.cpp
void finalize(graph1* graph, int* &v_per_p, int* &v_per_p_begin);

#endif