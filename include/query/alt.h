/*
 * @(#)alt.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef ALT_H
#define ALT_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

void
alt_preprocess(heap_file*    hf,
               direction_t   d,
               unsigned long num_landmarks,
               dict_ul_d**   landmark_dists
#ifdef VERBOSE
               ,
               FILE* log_file
#endif
);

path*
alt(heap_file*    hf,
    dict_ul_d**   landmark_dists,
    unsigned long num_landmarks,
    unsigned long source_node_id,
    unsigned long target_node_id,
    direction_t   direction
#ifdef VERBOSE
    ,
    FILE* log_file
#endif
);

#endif
