/*!
 * \file dfs.h
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
#ifndef DFS_H
#define DFS_H

#include "access/heap_file.h"
#include "access/relationship.h"
#include "result_types.h"

traversal_result*
dfs(heap_file*    hf,
    unsigned long source_node_id,
    direction_t   direction,
    bool          log,
    FILE*         log_file);

#endif
