#include "../../src/locality/icbl/icbl.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/constants.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/import/snap_importer.h"

void
test_id_diff_sets(in_memory_file_t* db)
{
    dict_ul_ul_t** dif_sets =
          malloc(db->node_id_counter * sizeof(dict_ul_ul_t*));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dif_sets[i] = create_dict_ul_ul();
    }

    identify_diffustion_sets(db, dif_sets);

    size_t num_steps = get_num_steps(db);
    // unsigned long num_walks = get_num_walks(db);

    unsigned long* key      = NULL;
    unsigned long* value_a  = NULL;
    unsigned long  step_sum = 0;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dict_ul_ul_iterator_t* it = create_dict_ul_ul_iterator(dif_sets[i]);

        while (dict_ul_ul_iterator_next(it, &key, &value_a) == 0) {
            step_sum += *value_a;
            printf("Node: %lu, Visited: %lu\n", *key, *value_a);
        }
        dict_ul_ul_iterator_destroy(it);
        printf("i %lu, dict size: %lu Step sum: %lu, n steps %lu \n",
               i,
               dict_ul_ul_size(dif_sets[i]),
               step_sum,
               num_steps);
        // assert(step_sum == (num_steps + 1) * num_walks);
        step_sum = 0;
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dict_ul_ul_destroy(dif_sets[i]);
    }

    free(dif_sets);
}

// void test_weighted_jaccard_distance(in_memory_file_t* db) {

//}
//
// htable iterator
// void test_insert_match()
//
// subgraph distance
//
// map to partition
//
//

int
main(void)
{
    printf("Start importing\n");
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");
    dict_ul_ul_destroy(map);

    printf("Start applying the ICBL layout algorithm.\n");
    unsigned long* partition = icbl(db);
    printf("Done.\n");

    FILE* out_f =
          fopen("/home/someusername/workspace_local/icbl_layout.txt", "w");

    if (!out_f) {
        printf("Couldn't open file");
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        fprintf(out_f, "%lu %lu \n", i, partition[i]);
    }

    fclose(out_f);
    free(partition);

    test_id_diff_sets(db);

    in_memory_file_destroy(db);
    return 0;
}
