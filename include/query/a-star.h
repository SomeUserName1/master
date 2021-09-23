/*
 * @(#)a-star.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef A_STAR_H
#define A_STAR_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

path*
a_star(heap_file*    hf,
       dict_ul_d*    heuristic,
       unsigned long source_node_id,
       unsigned long target_node_id,
       direction_t   direction,
       bool          log,
       FILE*         log_file);

#endif
