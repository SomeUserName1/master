#ifndef RANDOM_WALK_H
#define RANDOM_WALK_H

#include <stddef.h>

#include "access/operators.h"
#include "result_types.h"

path*
random_walk(in_memory_file_t* db,
            unsigned long     node_id,
            size_t            num_steps,
            direction_t       direction);

#endif
