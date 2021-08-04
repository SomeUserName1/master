#ifndef DEGREE_H
#define DEGREE_H

#include <stdio.h>

#include "access/heap_file.h"
#include "access/relationship.h"

size_t
get_degree(heap_file*    hf,
           unsigned long node_id,
           direction_t   direction
#ifdef VERBOSE
           ,
           FILE* log_file
#endif
);

float
get_avg_degree(heap_file*  hf,
               direction_t direction
#ifdef VERBOSE
               ,
               FILE* log_file
#endif
);

size_t
get_min_degree(heap_file*  hf,
               direction_t direction
#ifdef VERBOSE
               ,
               FILE* log_file
#endif
);

size_t
get_max_degree(heap_file*  hf,
               direction_t direction
#ifdef VERBOSE
               ,
               FILE* log_file
#endif
);

#endif
