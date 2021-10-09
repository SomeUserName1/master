/*!
 * \file random_order.h
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
#ifndef RANDOM_LAYOUT_H
#define RANDOM_LAYOUT_H

#include "access/heap_file.h"

dict_ul_ul*
identity_node_order(heap_file* hf);

dict_ul_ul*
identity_relationship_order(heap_file* hf);

dict_ul_ul*
random_node_order(heap_file* hf);

dict_ul_ul*
random_relationship_order(heap_file* hf);

#endif
