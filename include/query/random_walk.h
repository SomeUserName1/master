#ifndef RANDOM_WALK_H
#define RANDOM_WALK_H

#include <stddef.h>

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

path*
random_walk(heap_file*    hf,
            unsigned long node_id,
            size_t        num_steps,
            direction_t   direction
#ifdef VERBOSE
            ,
            FILE* log_file
#endif
);

#endif
