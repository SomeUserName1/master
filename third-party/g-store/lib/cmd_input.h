#ifndef CMD_INPUT_H
#define CMD_INPUT_H

#include <stdio.h>

void read_from_input();
void read_menu();
void evaluate_menu(char* line);
void read_file(FILE* fp);
void read_from_input_query();
void cmd_misc();
void read_argument(char* line);
void display_help();
void display_describe(); 

#endif