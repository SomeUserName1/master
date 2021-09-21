/*
 * @(#)reorganize_records_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "layout/reorganize_records.h"

#include <assert.h>
#include <stdlib.h>

#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "query/snap_importer.h"

void
test_prepare_move_node(void)
{}

void
test_prepare_move_relationship(void)
{}

void
test_swap_nodes(void)
{}

void
test_swap_relationships(void)
{}

void
test_swap_page(void)
{}

void
test_reorder_nodes(void)
{}

void
test_reorder_relationships_by_ids(void)
{}

void
test_reorder_relationships_by_nodes(void)
{}

void
test_sort_incidence_array_list(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* degrees = calloc(db->node_id_counter, sizeof(unsigned long));
    array_list_ul** incidence_array_lists =
          calloc(db->node_id_counter, sizeof(array_list_ul*));

    array_list_relationship* rels;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels                     = in_memory_expand(db, i, BOTH);
        incidence_array_lists[i] = al_ul_create();

        degrees[i] = array_list_relationship_size(rels);
        for (size_t j = 0; j < degrees[i]; ++j) {
            array_list_ul_append(incidence_array_lists[i],
                                 array_list_relationship_get(rels, j)->id);
        }
        array_list_relationship_destroy(rels);
    }

    sort_incidence_list(db);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);

        for (size_t j = 0; j < degrees[i]; ++j) {
            assert(array_list_ul_contains(
                  incidence_array_lists[i],
                  array_list_relationship_get(rels, j)->id));
        }
        array_list_relationship_destroy(rels);
        array_list_ul_destroy(incidence_array_lists[i]);
    }
    free(incidence_array_lists);
    free(degrees);
    in_memory_file_destroy(db);
}

int
main(void)
{
    test_remap_node_ids();
    test_remap_rel_ids();
    test_sort_incidence_array_list();
    test_prepare_move_node();
    printf("finished test prepare_move_node\n");
    test_prepare_move_relationship();
    printf("finished test prepare_move_relationship\n");
    test_swap_page();
}
