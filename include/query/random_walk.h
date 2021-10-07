/*!
 * \file random_walk.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief TODO
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
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
            direction_t   direction,
            bool          log,
            FILE*         log_file);

#endif
