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

heap_file*
prepare(void)
{
#ifdef VERBOSE
    log_file = fopen(log_path, "w+");
    if (!log_file) {
        printf("ALT test: failed to open log file! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif

    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_phf   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";
#endif

    phy_database* phf = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_phf
#endif
    );

    page_cache* pc = page_cache_create(phf,
                                       CACHE_N_PAGES
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    heap_file* hf = heap_file_create(pc
#ifdef VERBOSE
                                     ,
                                     log_name_file
#endif
    );

    import(hf, false, C_ELEGANS);

    return hf;
}

void
clean_up(heap_file* hf)
{
    page_cache*   pc  = hf->cache;
    phy_database* pdb = pc->pdb;
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

#ifdef VERBOSE
    fclose(log_file);
#endif
}

void
test_prepare_move_node(void)
{
    heap_file* hf = prepare();

    array_list_relationship* rels = expand(hf, 0, BOTH);
    bool                     is_src_node[array_list_relationship_size(rels)];

    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        is_src_node[i] = array_list_relationship_get(rels, i)->source_node == 0;
    }
    array_list_relationship_destroy(rels);

    prepare_move_node(hf, 0, 1);

    rels = expand(hf, 0, BOTH);
    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        assert(is_src_node[i]
                     && array_list_relationship_get(rels, i)->source_node == 1
               || array_list_relationship_get(rels, i)->target_node == 1);
    }
    array_list_relationship_destroy(rels);

    clean_up(hf);
}

void
test_prepare_move_relationship(void)
{
    heap_file* hf = prepare();

    relationship_t* rel       = read_relationship(hf, 0);
    relationship_t* prev_src  = read_relationship(hf, rel->prev_rel_source);
    relationship_t* next_src  = read_relationship(hf, rel->next_rel_source);
    relationship_t* prev_trgt = read_relationship(hf, rel->prev_rel_target);
    relationship_t* next_trgt = read_relationship(hf, rel->next_rel_target);

    bool which_pointer[4][4];
    memset(which_pointer, 0, 4 * sizeof(unsigned char));

    if (prev_src->prev_rel_source == 0) {
        which_pointer[0][0] = true;
    }
    if (prev_src->next_rel_source == 0) {
        which_pointer[0][1] = true;
    }
    if (prev_src->prev_rel_target == 0) {
        which_pointer[0][2] = true;
    }
    if (prev_src->next_rel_target == 0) {
        which_pointer[0][3] = true;
    }

    if (next_src->prev_rel_source == 0) {
        which_pointer[1][0] = true;
    }
    if (next_src->next_rel_source == 0) {
        which_pointer[1][1] = true;
    }
    if (next_src->prev_rel_target == 0) {
        which_pointer[1][2] = true;
    }
    if (next_src->next_rel_target == 0) {
        which_pointer[1][3] = true;
    }

    if (prev_trgt->prev_rel_source == 0) {
        which_pointer[2][0] = true;
    }
    if (prev_trgt->next_rel_source == 0) {
        which_pointer[2][1] = true;
    }
    if (prev_trgt->prev_rel_target == 0) {
        which_pointer[2][2] = true;
    }
    if (prev_trgt->next_rel_target == 0) {
        which_pointer[2][3] = true;
    }

    if (next_trgt->prev_rel_source == 0) {
        which_pointer[3][0] = true;
    }
    if (next_trgt->next_rel_source == 0) {
        which_pointer[3][1] = true;
    }
    if (next_trgt->prev_rel_target == 0) {
        which_pointer[3][2] = true;
    }
    if (next_trgt->next_rel_target == 0) {
        which_pointer[3][3] = true;
    }

    prepare_move_relationship(hf, 0, 1);

    if (which_pointer[0][0]) {
        assert(prev_src->prev_rel_source == 1);
    }
    if (which_pointer[0][1]) {
        assert(prev_src->next_rel_source == 1);
    }
    if (which_pointer[0][2]) {
        assert(prev_src->prev_rel_target == 1);
    }
    if (which_pointer[0][3]) {
        assert(prev_src->next_rel_target == 1);
    }

    if (which_pointer[1][0]) {
        assert(next_src->prev_rel_source == 1);
    }
    if (which_pointer[1][1]) {
        assert(next_src->next_rel_source == 1);
    }
    if (which_pointer[1][2]) {
        assert(next_src->prev_rel_target == 1);
    }
    if (which_pointer[1][3]) {
        assert(next_src->next_rel_target == 1);
    }

    if (which_pointer[2][0]) {
        assert(prev_trgt->prev_rel_source == 1);
    }
    if (which_pointer[2][1]) {
        assert(prev_trgt->next_rel_source == 1);
    }
    if (which_pointer[2][2]) {
        assert(prev_trgt->prev_rel_target == 1);
    }
    if (which_pointer[2][3]) {
        assert(prev_trgt->next_rel_target == 1);
    }

    if (which_pointer[3][0]) {
        assert(prev_src->prev_rel_source == 1);
    }
    if (which_pointer[3][1]) {
        assert(prev_src->next_rel_source == 1);
    }
    if (which_pointer[3][2]) {
        assert(prev_src->prev_rel_target == 1);
    }
    if (which_pointer[3][3]) {
        assert(prev_src->next_rel_target == 1);
    }

    node_t* fst_src;
    node_t* fst_trgt;
    if ((rel->flags & FIRST_REL_SOURCE_FLAG) != 0) {
        fst_src = read_node(hf, rel->source_node);
        assert(fst_src->first_relationship == 1);
    }
    if ((rel->flags & FIRST_REL_TARGET_FLAG) != 0) {
        fst_trgt = read_node(hf, rel->target_node);
        assert(fst_trgt->first_relationship == 1);
    }

    clean_up(hf);
}

void
test_swap_nodes(void)
{}

void
test_swap_relationships(void)
{}

void
test_swap_record_pages(void)
{}

void
test_reorder_nodes(void)
{}

void
test_reorder_nodes_by_sequence(void)
{}

void
test_reorder_relationships(void)
{}

void
test_reorder_relationship_by_sequence(void)
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
    test_prepare_move_node();
    printf("finished test prepare_move_node\n");
    test_prepare_move_relationship();
    printf("finished test prepare_move_relationship\n");
    test_swap_nodes();
    printf("finished test swap nodes\n");
    test_swap_relationships();
    printf("finished test swap relationships\n");
    test_swap_record_pages();
    printf("finished test swap record pages\n");
    test_reorder_nodes();
    printf("finished test reorder nodes\n");
    test_reorder_nodes_by_sequence();
    printf("finished test reorder nodes by sequence\n");
    test_reorder_relationships();
    printf("finished test reorder relationships\n");
    test_reorder_relationships_by_nodes();
    printf("finished test reorder relationships by nodes\n");
    test_reorder_relationship_by_sequence();
    printf("finished test reorder relationships by sequence\n");
    test_sort_incidence_array_list();
    printf("finished test sort incidence list\n");
}
