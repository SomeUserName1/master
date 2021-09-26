/*
 * @(#)reorganize_records_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "constants.h"
#include "layout/reorganize_records.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"
#include "query/snap_importer.h"

heap_file*
prepare(void)
{

    char* file_name = "test";

    char* log_name_phf   = "log_test_pdb";
    char* log_name_cache = "log_test_pc";
    char* log_name_file  = "log_test_hf";

    phy_database* phf = phy_database_create(file_name, log_name_phf);

    page_cache* pc = page_cache_create(phf, CACHE_N_PAGES, log_name_cache);

    heap_file* hf = heap_file_create(pc, log_name_file);

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
}

void
test_prepare_move_node(void)
{
    heap_file* hf = prepare();

    array_list_relationship* rels = expand(hf, 0, BOTH, false);
    bool                     is_src_node[array_list_relationship_size(rels)];

    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        is_src_node[i] = array_list_relationship_get(rels, i)->source_node == 0;
    }
    array_list_relationship_destroy(rels);

    prepare_move_node(hf, 0, 1, true);

    rels = expand(hf, 0, BOTH, false);
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

    relationship_t* rel = read_relationship(hf, 0, false);
    relationship_t* prev_src =
          read_relationship(hf, rel->prev_rel_source, false);
    relationship_t* next_src =
          read_relationship(hf, rel->next_rel_source, false);
    relationship_t* prev_trgt =
          read_relationship(hf, rel->prev_rel_target, false);
    relationship_t* next_trgt =
          read_relationship(hf, rel->next_rel_target, false);

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

    prepare_move_relationship(hf, 0, 1, false);

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

    node_t* fst_src = read_node(hf, rel->source_node, false);
    ;
    node_t* fst_trgt = read_node(hf, rel->target_node, false);
    if (fst_src->first_relationship == rel->id) {
        assert(fst_src->first_relationship == 1);
    }
    if (fst_trgt->first_relationship == rel->id) {
        assert(fst_trgt->first_relationship == 1);
    }

    clean_up(hf);
}

void
test_swap_nodes(void)
{
    heap_file* hf = prepare();

    static const unsigned long id_on_snd_page = 90;

    node_t* node_1_before = read_node(hf, 1, false);
    node_t* node_2_before = read_node(hf, id_on_snd_page, false);

    swap_nodes(hf, 1, id_on_snd_page, true);

    node_t* node_1_after = read_node(hf, 1, false);
    node_t* node_2_after = read_node(hf, id_on_snd_page, false);

    assert(node_1_before->label == node_2_after->label);
    assert(node_1_before->first_relationship
           == node_2_after->first_relationship);
    assert(node_1_before->id == node_1_after->id);

    assert(node_2_before->label == node_1_after->label);
    assert(node_2_before->first_relationship
           == node_1_after->first_relationship);
    assert(node_2_before->id == node_2_after->id);

    static const unsigned long id_on_third_page = 180;

    delete_node(hf, id_on_third_page, false);

    swap_nodes(hf, id_on_snd_page, id_on_third_page, false);

    node_t* node_3 = read_node(hf, id_on_third_page, false);

    assert(node_2_after->label == node_3->label);
    assert(node_2_after->first_relationship == node_3->first_relationship);

    assert(!check_record_exists(hf, id_on_snd_page, true, false));
    assert(check_record_exists(hf, id_on_third_page, true, false));

    clean_up(hf);
}

void
test_swap_relationships(void)
{
    heap_file* hf = prepare();

    static const unsigned long id_on_snd_page = 45;

    relationship_t* relationship_1_before = read_relationship(hf, 1, false);
    relationship_t* relationship_2_before =
          read_relationship(hf, id_on_snd_page, false);

    swap_relationships(hf, 1, id_on_snd_page, true);

    relationship_t* relationship_1_after = read_relationship(hf, 1, false);
    relationship_t* relationship_2_after =
          read_relationship(hf, id_on_snd_page, false);

    assert(relationship_1_before->label == relationship_2_after->label);

    assert(relationship_1_before->source_node
           == relationship_2_after->source_node);

    assert(relationship_1_before->target_node
           == relationship_2_after->target_node);

    assert(relationship_1_before->prev_rel_source
           == relationship_2_after->prev_rel_source);

    assert(relationship_1_before->next_rel_source
           == relationship_2_after->next_rel_source);

    assert(relationship_1_before->prev_rel_target
           == relationship_2_after->prev_rel_target);

    assert(relationship_1_before->next_rel_target
           == relationship_2_after->next_rel_target);

    assert(relationship_1_before->weight == relationship_2_after->weight);

    assert(relationship_2_before->label == relationship_1_after->label);

    assert(relationship_2_before->source_node
           == relationship_1_after->source_node);

    assert(relationship_2_before->target_node
           == relationship_1_after->target_node);

    assert(relationship_2_before->prev_rel_source
           == relationship_1_after->prev_rel_source);

    assert(relationship_2_before->next_rel_source
           == relationship_1_after->next_rel_source);

    assert(relationship_2_before->prev_rel_target
           == relationship_1_after->prev_rel_target);

    assert(relationship_2_before->next_rel_target
           == relationship_1_after->next_rel_target);

    assert(relationship_2_before->weight == relationship_1_after->weight);

    assert(relationship_2_before->id == relationship_2_after->id);

    static const unsigned long id_on_third_page = 180;

    delete_relationship(hf, id_on_third_page, false);

    swap_relationships(hf, id_on_snd_page, id_on_third_page, false);

    relationship_t* relationship_3 =
          read_relationship(hf, id_on_third_page, false);

    assert(relationship_2_after->label == relationship_3->label);

    assert(relationship_2_after->source_node == relationship_3->source_node);

    assert(relationship_2_after->target_node == relationship_3->target_node);

    assert(relationship_2_after->prev_rel_source
           == relationship_3->prev_rel_source);

    assert(relationship_2_after->next_rel_source
           == relationship_3->next_rel_source);

    assert(relationship_2_after->prev_rel_target
           == relationship_3->prev_rel_target);

    assert(relationship_2_after->next_rel_target
           == relationship_3->next_rel_target);

    assert(relationship_2_after->weight == relationship_3->weight);

    assert(relationship_2_after->id == relationship_2_after->id);

    assert(!check_record_exists(hf, id_on_snd_page, false, false));
    assert(check_record_exists(hf, id_on_third_page, false, false));

    clean_up(hf);
}

void
test_swap_record_pages(void)
{
    heap_file* hf = prepare();

    page*          header_page = pin_page(hf->cache, 0, header, node_ft, false);
    unsigned char* header_copy = malloc(PAGE_SIZE);
    memcpy(header_copy, header_page->data, PAGE_SIZE);
    unpin_page(hf->cache, 0, header, node_ft, false);

    page*          first_page = pin_page(hf->cache, 0, records, node_ft, false);
    unsigned char* first_page_copy = malloc(PAGE_SIZE);
    memcpy(first_page_copy, first_page->data, PAGE_SIZE);
    unpin_page(hf->cache, 0, records, node_ft, false);

    page* second_page = pin_page(hf->cache, 1, records, node_ft, false);
    unsigned char* second_page_copy = malloc(PAGE_SIZE);
    memcpy(second_page_copy, second_page->data, PAGE_SIZE);
    unpin_page(hf->cache, 1, records, node_ft, false);

    static const size_t last_node_id = 127;
    static const size_t f_s_id       = 128;
    static const size_t l_s_id       = 255;
    node_t*             fst_node     = read_node(hf, 0, false);
    node_t*             lst_node     = read_node(hf, last_node_id, false);
    node_t*             f_s_node     = read_node(hf, f_s_id, false);
    node_t*             l_s_node     = read_node(hf, l_s_id, false);

    swap_record_pages(hf, 0, 1, node_ft, true);

    header_page = pin_page(hf->cache, 0, header, node_ft, false);
    for (size_t i = 0; i < 2 * SLOTS_PER_PAGE / CHAR_BIT; ++i) {
        if (i < SLOTS_PER_PAGE / CHAR_BIT) {
            assert(header_page->data[i]
                   == header_copy[SLOTS_PER_PAGE / CHAR_BIT + i]);
        } else {
            assert(header_page->data[SLOTS_PER_PAGE / CHAR_BIT + i]
                   == header_copy[i]);
        }
    }
    unpin_page(hf->cache, 0, header, node_ft, false);

    node_t* fst_node_a = read_node(hf, 0, false);
    node_t* lst_node_a = read_node(hf, last_node_id, false);
    node_t* f_s_node_a = read_node(hf, f_s_id, false);
    node_t* l_s_node_a = read_node(hf, l_s_id, false);

    assert(fst_node->first_relationship == f_s_node_a->first_relationship);
    assert(fst_node->label == f_s_node_a->label);

    assert(lst_node->first_relationship == l_s_node_a->first_relationship);
    assert(lst_node->label == l_s_node_a->label);

    assert(f_s_node->first_relationship == fst_node_a->first_relationship);
    assert(f_s_node->label == fst_node_a->label);

    assert(l_s_node->first_relationship == lst_node_a->first_relationship);
    assert(l_s_node->label == lst_node_a->label);

    clean_up(hf);
}

void
test_reorder_nodes(void)
{
    heap_file* hf = prepare();

    array_list_node* nodes   = get_nodes(hf, false);
    dict_ul_ul*      new_ids = d_ul_ul_create();

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        dict_ul_ul_insert(new_ids,
                          array_list_node_get(nodes, i)->id,
                          array_list_node_get(nodes, hf->n_nodes - i - 1)->id);
    }

    reorder_nodes(hf, new_ids, NULL, true);

    array_list_node* new_nodes = get_nodes(hf, false);

    assert(array_list_node_size(nodes) == array_list_node_size(new_nodes));

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        assert(array_list_node_get(nodes, i)->first_relationship
               == array_list_node_get(new_nodes, hf->n_nodes - i - 1)
                        ->first_relationship);

        assert(array_list_node_get(nodes, i)->label
               == array_list_node_get(new_nodes, hf->n_nodes - i - 1)->label);
    }

    array_list_node_destroy(nodes);
    array_list_node_destroy(new_nodes);

    clean_up(hf);
}

void
test_reorder_nodes_by_sequence(void)
{
    heap_file* hf = prepare();

    array_list_node* nodes = get_nodes(hf, false);

    unsigned long sequence[hf->n_nodes];

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        sequence[i] = array_list_node_get(nodes, hf->n_nodes - i - 1)->id;
    }

    reorder_nodes_by_sequence(hf, sequence, true);

    array_list_node* new_nodes = get_nodes(hf, false);

    assert(array_list_node_size(nodes) == array_list_node_size(new_nodes));

    for (size_t i = 0; i < hf->n_nodes; ++i) {
        assert(array_list_node_get(nodes, i)->first_relationship
               == array_list_node_get(new_nodes, hf->n_nodes - i - 1)
                        ->first_relationship);

        assert(array_list_node_get(nodes, i)->label
               == array_list_node_get(new_nodes, hf->n_nodes - i - 1)->label);
    }

    array_list_node_destroy(nodes);
    array_list_node_destroy(new_nodes);

    clean_up(hf);
}

void
test_reorder_relationships(void)
{
    heap_file* hf = prepare();

    array_list_relationship* rels    = get_relationships(hf, false);
    dict_ul_ul*              new_ids = d_ul_ul_create();

    for (size_t i = 0; i < hf->n_rels; ++i) {
        dict_ul_ul_insert(
              new_ids,
              array_list_relationship_get(rels, i)->id,
              array_list_relationship_get(rels, hf->n_rels - i - 1)->id);
    }

    reorder_relationships(hf, new_ids, NULL, true);

    array_list_relationship* new_rels = get_relationships(hf, false);

    assert(array_list_relationship_size(rels)
           == array_list_relationship_size(new_rels));

    for (size_t i = 0; i < hf->n_rels; ++i) {
        assert(array_list_relationship_get(rels, i)->source_node
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->source_node);

        assert(array_list_relationship_get(rels, i)->target_node
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->target_node);

        assert(array_list_relationship_get(rels, i)->prev_rel_source
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->prev_rel_source);

        assert(array_list_relationship_get(rels, i)->next_rel_source
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->next_rel_source);

        assert(array_list_relationship_get(rels, i)->prev_rel_target
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->prev_rel_target);

        assert(array_list_relationship_get(rels, i)->next_rel_target
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->next_rel_target);

        assert(array_list_relationship_get(rels, i)->weight
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->weight);

        assert(array_list_relationship_get(rels, i)->label
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->label);
    }

    array_list_relationship_destroy(rels);
    array_list_relationship_destroy(new_rels);

    clean_up(hf);
}

void
test_reorder_relationship_by_sequence(void)
{
    heap_file* hf = prepare();

    array_list_relationship* rels = get_relationships(hf, false);
    unsigned long            sequence[hf->n_rels];

    for (size_t i = 0; i < hf->n_rels; ++i) {
        sequence[i] = array_list_relationship_get(rels, hf->n_rels - i - 1)->id;
    }

    reorder_relationships_by_sequence(hf, sequence, true);

    array_list_relationship* new_rels = get_relationships(hf, false);

    assert(array_list_relationship_size(rels)
           == array_list_relationship_size(new_rels));

    for (size_t i = 0; i < hf->n_rels; ++i) {
        assert(array_list_relationship_get(rels, i)->source_node
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->source_node);

        assert(array_list_relationship_get(rels, i)->target_node
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->target_node);

        assert(array_list_relationship_get(rels, i)->prev_rel_source
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->prev_rel_source);

        assert(array_list_relationship_get(rels, i)->next_rel_source
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->next_rel_source);

        assert(array_list_relationship_get(rels, i)->prev_rel_target
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->prev_rel_target);

        assert(array_list_relationship_get(rels, i)->next_rel_target
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->next_rel_target);

        assert(array_list_relationship_get(rels, i)->weight
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->weight);

        assert(array_list_relationship_get(rels, i)->label
               == array_list_relationship_get(new_rels, hf->n_rels - i - 1)
                        ->label);
    }

    array_list_relationship_destroy(rels);
    array_list_relationship_destroy(new_rels);

    clean_up(hf);
}

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
