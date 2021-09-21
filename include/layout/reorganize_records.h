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

void
prepare_move_node(heap_file* hf, unsigned long id, unsigned long to_id);

void
prepare_move_relationship(heap_file* hf, unsigned long id, unsigned long to_id);

void
swap_nodes(heap_file* hf, unsigned long fst, unsigned long snd);

void
swap_relationships(heap_file* hf, unsigned long fst, unsigned long snd);

void
swap_page(heap_file* hf, size_t fst, size_t snd, file_type ft);

void
reorder_nodes(heap_file* hf, dict_ul_ul* new_ids);

void
reorder_relationships_by_ids(heap_file* hf, dict_ul_ul* new_ids);

void
reorder_relationships_by_nodes(heap_file* hf);

void
sort_incidence_list(heap_file* hf);

#endif
