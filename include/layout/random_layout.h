/*
 * @(#)random_layout.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef RANDOM_LAYOUT_H
#define RANDOM_LAYOUT_H

#include "access/heap_file.h"

dict_ul_ul*
identity_order(heap_file* hf);

dict_ul_ul*
random_order(heap_file* hf);

#endif
