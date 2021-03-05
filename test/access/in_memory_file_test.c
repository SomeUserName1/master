#include "../../src/access/in_memory_file.h"
#include "../../src/import/snap_importer.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/record/node.h"
#include "../../src/record/relationship.h"
#include "../../src/constants.h"

#include <assert.h>
#include <stdio.h>


void test_create_rel_chain(in_memory_file_t* db, dict_ul_ul_t* map) {
    node_t* node = in_memory_get_node(db, 0);
    relationship_t* rel = in_memory_get_relationship(db, node->first_relationship);
    // 1.
    assert(rel->id == 0);
    assert(rel->source_node == dict_ul_ul_get_direct(map, 0));
    assert(rel->target_node == dict_ul_ul_get_direct(map, 1));
    assert(rel->prev_rel_source == UNINITIALIZED_LONG);
    assert(rel->next_rel_source == 411);
    assert(rel->prev_rel_target == UNINITIALIZED_LONG);
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
    // 73.
}


int main(void) {
    in_memory_file_t* db = create_in_memory_file();
    printf("Import\n");
    dict_ul_ul_t* map = import_from_txt(db, "/home/someusername/workspace_local/email_eu.txt");

    test_create_rel_chain(db, map);

    in_memory_file_destroy(db);
    dict_ul_ul_destroy(map);
    return 0;
}
