#ifndef PRINT_GRAPH_H
#define PRINT_GRAPH_H

#include "defs.h"

void print_graph1(graph1 *graph, char* text, bool print_coarser);
void print_graph2(graph1 *graph, char* text, bool print_coarser);
void print_graph3(graph1 *graph, int* v_per_p, int* v_per_p_begin, char* text);
void print_part_gid(graph1 *graph, FILE* part_file, FILE* gid_map_file);

#endif