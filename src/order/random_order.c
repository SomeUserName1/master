/*!
 * \file random_order.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref random_order.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "order/random_order.h"
#include "access/node.h"
#include "data-struct/htable.h"

#include <stdio.h>
#include <stdlib.h>

dict_ul_ul*
identity_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("random orders - identity: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul*      order = d_ul_ul_create();
    array_list_node* nodes = get_nodes(hf, false);

    for (size_t i = 0; i < array_list_node_size(nodes); ++i) {
        dict_ul_ul_insert(order,
                          array_list_node_get(nodes, i)->id,
                          array_list_node_get(nodes, i)->id);
    }
    array_list_node_destroy(nodes);

    return order;
}

dict_ul_ul*
random_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("random order - random order: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul*      order = identity_order(hf);
    array_list_node* nodes = get_nodes(hf, false);

    size_t        r_i;
    size_t        rand_id;
    unsigned long cur_id;
    for (size_t i = 0; i < dict_ul_ul_size(order); ++i) {
        r_i     = rand() % (hf->n_nodes);
        rand_id = array_list_node_get(nodes, r_i)->id;

        cur_id =
              dict_ul_ul_get_direct(order, array_list_node_get(nodes, i)->id);

        dict_ul_ul_insert(order, cur_id, dict_ul_ul_get_direct(order, rand_id));

        dict_ul_ul_insert(order, rand_id, cur_id);
    }

    array_list_node_destroy(nodes);

    return order;
}
