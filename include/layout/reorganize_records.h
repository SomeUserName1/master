/*
 * @(#)reorganize_records.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef REORGANIZE_RECORDS_H
#define REORGANIZE_RECORDS_H

#include "access/heap_file.h"

unsigned long*
remap_node_ids(heap_file* hf, const unsigned long* partition);

unsigned long*
remap_rel_ids(heap_file* hf);

void
sort_incidence_list(heap_file* hf);

void
prepare_move_node(heap_file* hf, unsigned long id, unsigned long to_id);

void
prepare_move_relationship(heap_file* hf, unsigned long id, unsigned long to_id);

void
swap_page(heap_file* hf, size_t fst, size_t snd, file_type ft);

#endif
