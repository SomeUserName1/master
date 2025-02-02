/*!
 * \file reorder_records.h
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
#ifndef REORGANIZE_RECORDS_H
#define REORGANIZE_RECORDS_H

#include "access/heap_file.h"
#include "disk_file.h"

void
swap_nodes(heap_file* hf, unsigned long fst, unsigned long snd, bool log);

void
swap_relationships(heap_file*    hf,
                   unsigned long fst,
                   unsigned long snd,
                   bool          log);

void
swap_record_pages(heap_file* hf,
                  size_t     fst,
                  size_t     snd,
                  file_type  ft,
                  bool       log);

void
reorder_nodes(heap_file* hf, dict_ul_ul* new_ids, bool log);

void
reorder_nodes_by_sequence(heap_file*           hf,
                          const unsigned long* sequence,
                          bool                 log);

void
reorder_relationships(heap_file* hf, dict_ul_ul* new_ids, bool log);

void
reorder_relationships_by_nodes(heap_file* hf, bool log);

void
reorder_relationships_by_sequence(heap_file*           hf,
                                  const unsigned long* sequence,
                                  bool                 log);

void
sort_incidence_list(heap_file* hf, bool log);

#endif
