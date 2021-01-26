#ifndef READ_INPUT_H
#define READ_INPUT_H

#include "defs.h"

void read_graph(graph1* graph, FILE* fp_in);
void read_parts_into_graph(graph1* graph, FILE* fp_parts);
void generate_parts_into_graph(graph1* graph);
void generate_rnd_parts_into_graph(graph1* graph);
int get_rec_len(char* &str0, int line_id);


#endif