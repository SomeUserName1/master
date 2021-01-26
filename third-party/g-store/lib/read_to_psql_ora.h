#ifndef READ_TO_PSQL_ORA_H
#define READ_TO_PSQL_ORA_H

#include <stdio.h>

void read_to_ora_postgres(FILE* fp_in, FILE* fp_out, bool ora);
void read_to_postgres_copy(FILE* fp_in, FILE* fp_out, FILE* fp_out_v, FILE* fp_out_e);

#endif