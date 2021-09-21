/*
 * @(#)random_layout.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "layout/random_layout.h"
#include "access/node.h"
#include "data-struct/htable.h"

#include <stdio.h>
#include <stdlib.h>

dict_ul_ul*
identity_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("trivial partitions - identity: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul*      partition = d_ul_ul_create();
    array_list_node* nodes     = get_nodes(hf);

    for (size_t i = 0; i < array_list_node_size(nodes); ++i) {
        dict_ul_ul_insert(partition, array_list_node_get(nodes, i)->id, i);
    }
    array_list_node_destroy(nodes);

    return partition;
}

dict_ul_ul*
random_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("trivial partitions - identity: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul* partition = identity_order(hf);

    size_t        pos;
    unsigned long temp;
    for (size_t i = 0; i < dict_ul_ul_size(partition); ++i) {
        pos  = rand() % (hf->n_nodes);
        temp = dict_ul_ul_get_direct(partition, i);
        dict_ul_ul_insert(partition, i, dict_ul_ul_get_direct(partition, pos));
        dict_ul_ul_insert(partition, pos, temp);
    }

    return partition;
}
