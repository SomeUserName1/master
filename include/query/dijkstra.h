/*
 * @(#)dijkstra.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

sssp_result*
dijkstra(heap_file*    hf,
         unsigned long source_node_id,
         direction_t   direction,
         bool          log,
         FILE*         log_file);

#endif
