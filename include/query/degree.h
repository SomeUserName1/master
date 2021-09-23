/*
 * @(#)degree.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef DEGREE_H
#define DEGREE_H

#include <stdio.h>

#include "access/heap_file.h"
#include "access/relationship.h"

size_t
get_degree(heap_file*    hf,
           unsigned long node_id,
           direction_t   direction,
           bool          log,
           FILE*         log_file);

float
get_avg_degree(heap_file*  hf,
               direction_t direction

               ,
               bool log,

               FILE* log_file);

size_t
get_min_degree(heap_file* hf, direction_t direction, bool log, FILE* log_file);

size_t
get_max_degree(heap_file* hf, direction_t direction, bool log, FILE* log_file);

#endif
