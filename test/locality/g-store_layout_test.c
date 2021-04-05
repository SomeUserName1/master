#include "../../src/locality/g-store/g-store_layout.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/constants.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/import/snap_importer.h"

void
test_uncoarsen(in_memory_file_t* db)
{
    multi_level_graph_t* graph = malloc(sizeof(*graph));
    multi_level_graph_t* prev;

    graph->c_level        = 0;
    graph->finer          = NULL;
    graph->coarser        = NULL;
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(unsigned long));
    graph->node_aggregation_weight =
          calloc(db->node_id_counter, sizeof(unsigned long));
    graph->records = db;

    size_t               num_v_matches = 2;
    float                c_ratio_avg   = 0.0F;
    unsigned int         i             = 0;
    list_relationship_t* rels_finer;
    relationship_t*      rel;
    relationship_t*      coarser_rel;
    unsigned long        other_id;
    unsigned long        max_partition_size = BLOCK_SIZE / sizeof(node_t);

    while (coarsen(graph,
                   BLOCK_SIZE,
                   &num_v_matches,
                   &max_partition_size,
                   &c_ratio_avg)
           == 0) {
        prev  = graph;
        graph = graph->coarser;

        assert(graph);
        assert(graph->finer == prev);

        assert(graph->records->node_id_counter
               < prev->records->node_id_counter);
        assert(graph->records->node_id_counter
               >= prev->records->node_id_counter / num_v_matches);

        assert(graph->records->rel_id_counter
               <= prev->records->rel_id_counter
                        - (prev->records->node_id_counter
                           - graph->records->node_id_counter));

        i++;
        assert(graph->c_level == i);

        for (size_t j = 0; j < prev->records->node_id_counter; ++j) {
            rels_finer = in_memory_expand(prev->records, j, BOTH);

            for (size_t k = 0; k < list_relationship_size(rels_finer); ++k) {
                rel = list_relationship_get(rels_finer, k);

                other_id = j == rel->source_node ? rel->target_node
                                                 : rel->source_node;

                if (prev->map_to_coarser[j] == prev->map_to_coarser[other_id]) {
                    continue;
                }

                coarser_rel = in_memory_contains_relationship_from_to(
                      graph->records,
                      prev->map_to_coarser[j],
                      prev->map_to_coarser[other_id],
                      BOTH);

                assert(coarser_rel);
            }
            list_relationship_destroy(rels_finer);
        }
    }

    assert(graph->coarser == NULL);

    while (graph->finer) {
        graph = graph->finer;
        free(graph->coarser->map_to_coarser);
        free(graph->coarser->node_aggregation_weight);
        in_memory_file_destroy(graph->coarser->records);
        free(graph->coarser);
    }

    assert(graph->finer == NULL);

    free(graph->map_to_coarser);
    free(graph->node_aggregation_weight);
    free(graph);
}

void
test_full_run(in_memory_file_t* db)
{
    printf("Start applying the G-Store multilevel partitioning algorithm.\n");
    unsigned long* partition = g_store_layout(db, BLOCK_SIZE);
    printf("Done.\n");

    FILE* out_f =
          fopen("/home/someusername/workspace_local/g-store_layout.txt", "w");

    if (!out_f) {
        printf("Couldn't open file");
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        fprintf(out_f, "%lu %lu\n", i, partition[i]);
    }

    fclose(out_f);
    free(partition);
}

int
main(void)
{
    printf("Start importing\n");
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");
    dict_ul_ul_destroy(map);

    test_uncoarsen(db);

    test_full_run(db);

    in_memory_file_destroy(db);
    return 0;
}
