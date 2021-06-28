#include "access/in_memory_file.h"

#include <assert.h>
#include <stdio.h>

#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "data-struct/htable.h"
#include "query/in_memory_operators.h"
#include "query/snap_importer.h"

#define NUM_NODES (10)
#define NUM_EDGES (9)

static const unsigned long rel_ids_n0[] = {
    0,     411,   2181,  2265,  2388,  2430,  3476,  3854,  4278,  4741,  5671,
    5742,  5744,  5751,  5768,  5826,  5871,  5875,  6301,  6618,  7100,  7960,
    8517,  8518,  8548,  9155,  9696,  10385, 10654, 10655, 10895, 10990, 11093,
    11096, 11102, 11120, 11121, 11147, 11149, 11555, 11562, 11585, 11687, 11695,
    12931, 13120, 13225, 13233, 13565, 14670, 14735, 15113, 15309, 16514, 16515,
    16528, 16907, 16911, 17094, 19095, 20155, 20281, 20282, 21485, 21497, 23641,
    23754, 23905, 24729, 25269, 25374, 25511
};

static const size_t ids_n0_out[] = { 0,  2,  4,  7,  8,  9,  11, 12, 15, 17, 18,
                                     19, 20, 22, 23, 25, 28, 30, 31, 33, 34, 35,
                                     36, 38, 39, 41, 42, 45, 46, 49, 51, 53, 54,
                                     57, 59, 60, 62, 63, 65, 66, 70 };

static const size_t ids_n0_inc[] = { 1,  3,  5,  6,  10, 13, 14, 16, 18, 21, 24,
                                     26, 27, 29, 32, 37, 40, 43, 44, 47, 48, 50,
                                     52, 55, 56, 58, 61, 64, 67, 68, 69, 71 };

void
test_create_rel_chain(in_memory_file_t* db, dict_ul_ul* map)
{
    node_t*         node = in_memory_get_node(db, 0);
    relationship_t* rel =
          in_memory_get_relationship(db, node->first_relationship);
    // 1.
    assert(rel->id == 0);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 1));
    assert(rel->prev_rel_source == 25511);
    assert(rel->next_rel_source == 411);
    assert(rel->prev_rel_target == 25223);
    assert(rel->next_rel_target == 225);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 2.
    assert(rel->id == 411);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 17));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 410);
    assert(rel->next_rel_source == 412);
    assert(rel->prev_rel_target == 0);
    assert(rel->next_rel_target == 2181);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 3.
    assert(rel->id == 2181);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 316));
    assert(rel->prev_rel_source == 411);
    assert(rel->next_rel_source == 2265);
    assert(rel->prev_rel_target == 1737);
    assert(rel->next_rel_target == 2265);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 4.
    assert(rel->id == 2265);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 316));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 2181);
    assert(rel->next_rel_source == 2982);
    assert(rel->prev_rel_target == 2181);
    assert(rel->next_rel_target == 2388);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 5.
    assert(rel->id == 2388);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 146));
    assert(rel->prev_rel_source == 2265);
    assert(rel->next_rel_source == 2430);
    assert(rel->prev_rel_target == 125);
    assert(rel->next_rel_target == 2430);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 6.
    assert(rel->id == 2430);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 146));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 2388);
    assert(rel->next_rel_source == 2434);
    assert(rel->prev_rel_target == 2388);
    assert(rel->next_rel_target == 3476);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 7.
    assert(rel->id == 3476);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 581));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 3350);
    assert(rel->next_rel_source == 3501);
    assert(rel->prev_rel_target == 2430);
    assert(rel->next_rel_target == 3854);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 8.
    assert(rel->id == 3854);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 268));
    assert(rel->prev_rel_source == 3476);
    assert(rel->next_rel_source == 4278);
    assert(rel->prev_rel_target == 3081);
    assert(rel->next_rel_target == 4375);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 9.
    assert(rel->id == 4278);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 581));
    assert(rel->prev_rel_source == 3854);
    assert(rel->next_rel_source == 4741);
    assert(rel->prev_rel_target == 4237);
    assert(rel->next_rel_target == 4327);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 10.
    assert(rel->id == 4741);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 221));
    assert(rel->prev_rel_source == 4278);
    assert(rel->next_rel_source == 5671);
    assert(rel->prev_rel_target == 4729);
    assert(rel->next_rel_target == 4742);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 11.
    assert(rel->id == 5671);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 218));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 5670);
    assert(rel->next_rel_source == 5672);
    assert(rel->prev_rel_target == 4741);
    assert(rel->next_rel_target == 5742);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 12.
    assert(rel->id == 5742);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 18));
    assert(rel->prev_rel_source == 5671);
    assert(rel->next_rel_source == 5744);
    assert(rel->prev_rel_target == 5439);
    assert(rel->next_rel_target == 5768);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 13.
    assert(rel->id == 5744);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 734));
    assert(rel->prev_rel_source == 5742);
    assert(rel->next_rel_source == 5751);
    assert(rel->prev_rel_target == 5345);
    assert(rel->next_rel_target == 5751);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 14.
    assert(rel->id == 5751);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 734));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 5744);
    assert(rel->next_rel_source == 8606);
    assert(rel->prev_rel_target == 5744);
    assert(rel->next_rel_target == 5768);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 15.
    assert(rel->id == 5768);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 18));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 5742);
    assert(rel->next_rel_source == 9364);
    assert(rel->prev_rel_target == 5751);
    assert(rel->next_rel_target == 5826);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 16.
    assert(rel->id == 5826);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 178));
    assert(rel->prev_rel_source == 5768);
    assert(rel->next_rel_source == 5871);
    assert(rel->prev_rel_target == 5441);
    assert(rel->next_rel_target == 5871);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 17.
    assert(rel->id == 5871);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 178));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 5826);
    assert(rel->next_rel_source == 6081);
    assert(rel->prev_rel_target == 5826);
    assert(rel->next_rel_target == 5875);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 18.
    assert(rel->id == 5875);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 380));
    assert(rel->prev_rel_source == 5871);
    assert(rel->next_rel_source == 6301);
    assert(rel->prev_rel_target == 5606);
    assert(rel->next_rel_target == 5935);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 19.
    assert(rel->id == 6301);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 5875);
    assert(rel->next_rel_source == 6618);
    assert(rel->prev_rel_target == 5875);
    assert(rel->next_rel_target == 6618);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 20.
    assert(rel->id == 6618);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 459));
    assert(rel->prev_rel_source == 6301);
    assert(rel->next_rel_source == 7100);
    assert(rel->prev_rel_target == 6433);
    assert(rel->next_rel_target == 6735);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 21.
    assert(rel->id == 7100);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 215));
    assert(rel->prev_rel_source == 6618);
    assert(rel->next_rel_source == 7960);
    assert(rel->prev_rel_target == 6839);
    assert(rel->next_rel_target == 7689);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 22.
    assert(rel->id == 7960);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 221));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 6983);
    assert(rel->next_rel_source == 8856);
    assert(rel->prev_rel_target == 7100);
    assert(rel->next_rel_target == 8517);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 23.
    assert(rel->id == 8517);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 250));
    assert(rel->prev_rel_source == 7960);
    assert(rel->next_rel_source == 8518);
    assert(rel->prev_rel_target == 8049);
    assert(rel->next_rel_target == 8548);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 24.
    assert(rel->id == 8518);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 148));
    assert(rel->prev_rel_source == 8517);
    assert(rel->next_rel_source == 8548);
    assert(rel->prev_rel_target == 6191);
    assert(rel->next_rel_target == 10196);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 25.
    assert(rel->id == 8548);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 250));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 8517);
    assert(rel->next_rel_source == 8723);
    assert(rel->prev_rel_target == 8518);
    assert(rel->next_rel_target == 9155);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 26.
    assert(rel->id == 9155);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 73));
    assert(rel->prev_rel_source == 8548);
    assert(rel->next_rel_source == 9696);
    assert(rel->prev_rel_target == 5786);
    assert(rel->next_rel_target == 9250);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 27.
    assert(rel->id == 9696);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 74));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 9695);
    assert(rel->next_rel_source == 9697);
    assert(rel->prev_rel_target == 9155);
    assert(rel->next_rel_target == 10385);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 28.
    assert(rel->id == 10385);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 248));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 10266);
    assert(rel->next_rel_source == 11555);
    assert(rel->prev_rel_target == 9696);
    assert(rel->next_rel_target == 10654);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 29.
    assert(rel->id == 10654);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 498));
    assert(rel->prev_rel_source == 10385);
    assert(rel->next_rel_source == 10655);
    assert(rel->prev_rel_target == 10504);
    assert(rel->next_rel_target == 10655);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 30.
    assert(rel->id == 10655);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 498));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 10654);
    assert(rel->next_rel_source == 10662);
    assert(rel->prev_rel_target == 10654);
    assert(rel->next_rel_target == 10895);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 31.
    assert(rel->id == 10895);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 226));
    assert(rel->prev_rel_source == 10655);
    assert(rel->next_rel_source == 10990);
    assert(rel->prev_rel_target == 10832);
    assert(rel->next_rel_target == 11106);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 32.
    assert(rel->id == 10990);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 101));
    assert(rel->prev_rel_source == 10895);
    assert(rel->next_rel_source == 11093);
    assert(rel->prev_rel_target == 10750);
    assert(rel->next_rel_target == 12640);

    rel = in_memory_get_relationship(db, rel->next_rel_source);
    // 33.
    assert(rel->id == 11093);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 377));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->prev_rel_source == 10715);
    assert(rel->next_rel_source == 11102);
    assert(rel->prev_rel_target == 10990);
    assert(rel->next_rel_target == 11096);

    rel = in_memory_get_relationship(db, rel->next_rel_target);
    // 34.
    assert(rel->id == 11096);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 218));
    assert(rel->prev_rel_source == 11093);
    assert(rel->next_rel_source == 11102);
    assert(rel->prev_rel_target == 11083);
    assert(rel->next_rel_target == 11106);

    // [...]
    // 72.
}

void
test_get_nodes(in_memory_file_t* db)
{
    size_t           n_nodes_before = db->node_id_counter;
    array_list_node* nodes          = in_memory_get_nodes(db);
    assert(array_list_node_size(nodes) == db->node_id_counter);
    array_list_node_destroy(nodes);
    assert(n_nodes_before == db->node_id_counter);
}

void
test_get_rels(in_memory_file_t* db)
{
    size_t                   n_rels_before = db->rel_id_counter;
    array_list_relationship* rels          = in_memory_get_relationships(db);
    assert(array_list_relationship_size(rels) == db->rel_id_counter);
    array_list_relationship_destroy(rels);
    assert(n_rels_before == db->rel_id_counter);
}

void
test_in_memory_next_rel(in_memory_file_t* db)
{
    relationship_t* rel = in_memory_get_relationship(db, 0);

    unsigned long next_rel_id =
          in_memory_next_relationship_id(db, 0, rel, OUTGOING);
    assert(next_rel_id == 2181);

    next_rel_id = in_memory_next_relationship_id(db, 0, rel, INCOMING);
    assert(next_rel_id == 411);

    next_rel_id = in_memory_next_relationship_id(db, 0, rel, BOTH);
    assert(next_rel_id == 411);

    next_rel_id = in_memory_next_relationship_id(db, 1, rel, OUTGOING);
    assert(next_rel_id == 2334);

    next_rel_id = in_memory_next_relationship_id(db, 1, rel, INCOMING);
    assert(next_rel_id == 225);

    next_rel_id = in_memory_next_relationship_id(db, 1, rel, BOTH);
    assert(next_rel_id == 225);
}

void
test_in_memory_next_rel_none(void)
{
    in_memory_file_t* db = create_in_memory_file();
    for (size_t i = 0; i < NUM_NODES; ++i) {
        in_memory_create_node(db);
    }

    in_memory_create_relationship(db, 0, 1);
    relationship_t* rel = in_memory_get_relationship(db, 0);

    unsigned long rel_id = in_memory_next_relationship_id(db, 0, rel, BOTH);
    assert(rel_id == UNINITIALIZED_LONG);

    in_memory_file_destroy(db);
}

void
test_in_memory_expand(in_memory_file_t* db)
{
    array_list_relationship* rels = in_memory_expand(db, 0, BOTH);
    assert(array_list_relationship_size(rels) == 72);

    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        assert(array_list_relationship_get(rels, i)->id == rel_ids_n0[i]);
    }
    array_list_relationship_destroy(rels);

    rels = in_memory_expand(db, 0, OUTGOING);
    assert(array_list_relationship_size(rels) == 41);

    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        assert(array_list_relationship_get(rels, i)->id
               == rel_ids_n0[ids_n0_out[i]]);
    }
    array_list_relationship_destroy(rels);

    rels = in_memory_expand(db, 0, INCOMING);
    assert(array_list_relationship_size(rels) == 32);

    for (size_t i = 0; i < array_list_relationship_size(rels); ++i) {
        assert(array_list_relationship_get(rels, i)->id
               == rel_ids_n0[ids_n0_inc[i]]);
    }
    array_list_relationship_destroy(rels);
}

void
test_in_memory_contains_rel(void)
{
    in_memory_file_t* db = create_in_memory_file();
    for (size_t i = 0; i < NUM_NODES; ++i) {
        in_memory_create_node(db);
    }

    for (size_t i = NUM_EDGES; i > 0; --i) {
        in_memory_create_relationship(db, i - 1, i);
    }

    relationship_t* rel;
    for (int i = 0; i < NUM_EDGES; ++i) {
        rel = in_memory_contains_relationship_from_to(db, i, i + 1, OUTGOING);
        assert(rel);

        rel = in_memory_contains_relationship_from_to(db, i, i + 1, INCOMING);
        assert(!rel);

        rel = in_memory_contains_relationship_from_to(db, i, i + 1, BOTH);
        assert(rel);
    }
    in_memory_file_destroy(db);
}

int
main(void)
{
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul*       map = import_from_txt(
          db, "/home/someusername/workspace_local/email_eu.txt");

    test_create_rel_chain(db, map);
    printf("Testing chains finished!\n");
    test_get_nodes(db);
    printf("Testing get_nodes finished!\n");
    test_get_rels(db);
    printf("Testing get rels finished!\n");
    test_in_memory_next_rel(db);
    printf("Testing rel iter finished!\n");
    test_in_memory_next_rel_none();
    printf("Testing rel iter in case of no further elements finished!\n");
    test_in_memory_expand(db);
    printf("Testing expand operator finished!\n");
    test_in_memory_contains_rel();
    printf("Testing contains rels finished!\n");

    in_memory_file_destroy(db);
    dict_ul_ul_destroy(map);
    return 0;
}
