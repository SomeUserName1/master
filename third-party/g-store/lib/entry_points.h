#ifndef ENTRY_POINTS_H
#define ENTRY_POINTS_H

#include "defs.h"

void create_db_new(FILE* fp_in);
void create_db_part(FILE* fp_in, FILE* fp_parts, char flat);
void gstore_init();
void close_down();

#endif