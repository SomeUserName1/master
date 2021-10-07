/*!
 * \file in_memory_graph.h
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
#ifndef IN_MEMORY_GRAPH_H
#define IN_MEMORY_GRAPH_H

#include "access/node.h"
#include "access/relationship.h"

typedef struct
{
    dict_ul_node* cache_nodes;
    dict_ul_rel*  cache_rels;
    unsigned long n_nodes;
    unsigned long n_rels;
} in_memory_graph;

in_memory_graph*
in_memory_graph_create(void);

void
in_memory_graph_destroy(in_memory_graph* db);

unsigned long
in_memory_create_node(in_memory_graph* db, unsigned long label);

unsigned long
in_memory_create_relationship(in_memory_graph* db,
                              unsigned long    node_from,
                              unsigned long    node_to,
                              unsigned long    label);
unsigned long
in_memory_create_relationship_weighted(in_memory_graph* db,
                                       unsigned long    node_from,
                                       unsigned long    node_to,
                                       double           weight,
                                       unsigned long    label);

node_t*
in_memory_get_node(in_memory_graph* db, unsigned long id);

relationship_t*
in_memory_get_relationship(in_memory_graph* db, unsigned long id);

inm_alist_node*
in_memory_get_nodes(in_memory_graph* db);

inm_alist_relationship*
in_memory_get_relationships(in_memory_graph* db);

unsigned long
in_memory_next_relationship_id(in_memory_graph* db,
                               unsigned long    node_id,
                               relationship_t*  rel,
                               direction_t      direction);
inm_alist_relationship*
in_memory_expand(in_memory_graph* db,
                 unsigned long    node_id,
                 direction_t      direction);
relationship_t*
in_memory_contains_relationship_from_to(in_memory_graph* db,
                                        unsigned long    node_from,
                                        unsigned long    node_to,
                                        direction_t      direction);

#endif
