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
#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/htable.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

dict_ul_ul*
identity_node_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("random orders - identity node order: Invalid Arguments!\n");
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
identity_relationship_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("random orders - identity relationship order: Invalid "
               "Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul*              order = d_ul_ul_create();
    array_list_relationship* rels  = get_relationships(hf, false);

    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        dict_ul_ul_insert(order,
                          array_list_relationship_get(rels, i)->id,
                          array_list_relationship_get(rels, i)->id);
    }
    array_list_relationship_destroy(rels);

    return order;
}

dict_ul_ul*
random_node_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf("random order - random node order: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul*      order = identity_node_order(hf);
    array_list_node* nodes = get_nodes(hf, false);

    size_t        r_i;
    size_t        rand_id;
    unsigned long cur_id;
    unsigned long cur_val;
    unsigned long rand_val;
    for (size_t i = 0; i < dict_ul_ul_size(order); ++i) {
        r_i     = rand() % (hf->n_nodes);
        rand_id = array_list_node_get(nodes, r_i)->id;
        cur_id  = array_list_node_get(nodes, i)->id;

        cur_val  = dict_ul_ul_get_direct(order, cur_id);
        rand_val = dict_ul_ul_get_direct(order, rand_id);

        dict_ul_ul_insert(order, cur_id, rand_val);
        dict_ul_ul_insert(order, rand_id, cur_val);
    }

    array_list_node_destroy(nodes);

    return order;
}

dict_ul_ul*
random_relationship_order(heap_file* hf)
{
    if (!hf) {
        // LCOV_EXCL_START
        printf(
              "random order - random relationship order: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    dict_ul_ul*              order = identity_relationship_order(hf);
    array_list_relationship* rels  = get_relationships(hf, false);

    size_t        r_i;
    size_t        rand_id;
    unsigned long cur_id;
    unsigned long cur_val;
    unsigned long rand_val;
    for (size_t i = 0; i < dict_ul_ul_size(order); ++i) {
        r_i     = rand() % (hf->n_rels);
        rand_id = array_list_relationship_get(rels, r_i)->id;
        cur_id  = array_list_relationship_get(rels, i)->id;

        cur_val  = dict_ul_ul_get_direct(order, cur_id);
        rand_val = dict_ul_ul_get_direct(order, rand_id);

        dict_ul_ul_insert(order, cur_id, rand_val);
        dict_ul_ul_insert(order, rand_id, cur_val);
    }

    array_list_relationship_destroy(rels);

    return order;
}
