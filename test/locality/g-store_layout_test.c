#include "../../src/locality/g-store_layout.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/constants.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/list_rel.h"
#include "../../src/data-struct/list_ul.h"
#include "../../src/import/snap_importer.h"
#include "../../src/record/node.h"
#include "../../src/record/relationship.h"

void
test_coarsen(in_memory_file_t* db)
{
    multi_level_graph_t* graph = malloc(sizeof(*graph));
    multi_level_graph_t* prev;

    if (!graph) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    graph->c_level        = 0;
    graph->finer          = NULL;
    graph->coarser        = NULL;
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(unsigned long));
    graph->node_aggregation_weight =
          calloc(db->node_id_counter, sizeof(unsigned long));

    if (!graph->map_to_coarser || !graph->node_aggregation_weight) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    graph->records = db;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        graph->node_aggregation_weight[i] = 1;
    }

    size_t               num_v_matches = 2;
    float                c_ratio_avg   = 0.0F;
    unsigned int         i             = 0;
    list_relationship_t* rels_finer;
    relationship_t*      rel;
    relationship_t*      coarser_rel;
    unsigned long        other_id;
    unsigned long        max_partition_size = BLOCK_SIZE / sizeof(node_t);
    unsigned long        node_aggregation_weight_sum;
    unsigned long*       finer_to_coarser_agg_sum;

    while (coarsen(graph, &num_v_matches, &max_partition_size, &c_ratio_avg)
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

        finer_to_coarser_agg_sum =
              calloc(graph->records->node_id_counter, sizeof(unsigned long));
        for (size_t i = 0; i < prev->records->node_id_counter; ++i) {
            finer_to_coarser_agg_sum[prev->map_to_coarser[i]] +=
                  prev->node_aggregation_weight[i];
        }

        node_aggregation_weight_sum = 0;
        for (size_t i = 0; i < graph->records->node_id_counter; ++i) {
            assert(finer_to_coarser_agg_sum[i]
                   == graph->node_aggregation_weight[i]);
            assert(graph->node_aggregation_weight[i] > 0);
            node_aggregation_weight_sum += graph->node_aggregation_weight[i];
        }
        assert(node_aggregation_weight_sum == db->node_id_counter);

        free(finer_to_coarser_agg_sum);
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
test_turn_arround(in_memory_file_t* db)
{
    multi_level_graph_t* graph = malloc(sizeof(*graph));

    if (!graph) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }
    graph->c_level        = 0;
    graph->finer          = NULL;
    graph->coarser        = NULL;
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(unsigned long));
    graph->node_aggregation_weight =
          calloc(db->node_id_counter, sizeof(unsigned long));

    if (!graph->map_to_coarser || !graph->node_aggregation_weight) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    graph->records = db;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        graph->node_aggregation_weight[i] = 1;
    }

    size_t        num_v_matches      = 2;
    float         c_ratio_avg        = 0.0F;
    unsigned long max_partition_size = BLOCK_SIZE / sizeof(node_t);

    while (coarsen(graph, &num_v_matches, &max_partition_size, &c_ratio_avg)
           == 0) {
        graph = graph->coarser;
    }

    turn_around(graph);

    unsigned long part_count[graph->num_partitions];

    for (size_t i = 0; i < graph->num_partitions; ++i) {
        part_count[i] = 0;
    }

    for (size_t i = 0; i < graph->records->node_id_counter; ++i) {
        part_count[graph->partition[i]]++;
    }

    for (size_t i = 0; i < graph->num_partitions; ++i) {
        assert(part_count[graph->partition[i]] < 2
               || graph->partition_aggregation_weight[graph->partition[i]]
                        < BLOCK_SIZE / sizeof(node_t));
    }

    free(graph->partition);
    free(graph->partition_aggregation_weight);

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
test_project(in_memory_file_t* db)
{
    multi_level_graph_t* graph = malloc(sizeof(*graph));

    if (!graph) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    float weight_threshold;

    graph->c_level        = 0;
    graph->finer          = NULL;
    graph->coarser        = NULL;
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(unsigned long));
    graph->node_aggregation_weight =
          calloc(db->node_id_counter, sizeof(unsigned long));

    if (!graph->map_to_coarser || !graph->node_aggregation_weight) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    graph->records = db;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        graph->node_aggregation_weight[i] = 1;
    }

    size_t         num_v_matches      = 2;
    float          c_ratio_avg        = 0.0F;
    unsigned long  max_partition_size = BLOCK_SIZE / sizeof(node_t);
    list_ul_t**    nodes_per_part;
    bool*          part_type;
    bool           zero;
    unsigned long* part_count;

    while (coarsen(graph, &num_v_matches, &max_partition_size, &c_ratio_avg)
           == 0) {
        graph = graph->coarser;
    }

    turn_around(graph);

    assert(graph->coarser == NULL);

    while (graph->finer) {
        part_type =
              calloc(graph->finer->records->node_id_counter, sizeof(bool));

        nodes_per_part = calloc(graph->num_partitions, sizeof(list_ul_t*));

        if (!part_type || !nodes_per_part) {
            printf("Allocating memory failed!\n");
            exit(-1);
        }

        for (size_t i = 0; i < graph->num_partitions; ++i) {
            nodes_per_part[i] = create_list_ul();
        }

        for (size_t i = 0; i < graph->finer->records->node_id_counter; ++i) {
            list_ul_append(
                  nodes_per_part
                        [graph->partition[graph->finer->map_to_coarser[i]]],
                  i);
        }

        project(graph, &part_type, c_ratio_avg, nodes_per_part);

        for (size_t i = 0; i < graph->num_partitions; ++i) {
            list_ul_destroy(nodes_per_part[i]);
        }
        free(nodes_per_part);

        assert(graph->finer->num_partitions > 0);
        if (graph->coarser != NULL) {
            assert(graph->finer->num_partitions >= graph->num_partitions);
        }

        part_count =
              calloc(graph->finer->num_partitions, sizeof(unsigned long));

        if (!part_count) {
            printf("Allocating memory failed!\n");
            exit(-1);
        }

        weight_threshold = ((float)BLOCK_SIZE / (float)sizeof(node_t))
                           / (float)pow(1 - c_ratio_avg, graph->finer->c_level);

        for (size_t i = 0; i < graph->finer->records->node_id_counter; ++i) {
            part_count[graph->finer->partition[i]] += 1;
        }

        zero = false;
        for (size_t i = 0; i < graph->finer->num_partitions; ++i) {
            if (!part_type[i]) {
                zero = true;
            } else {
                zero = false;
            }
            assert(part_count[i] > 0);
            assert(part_count[i] < 2
                   || graph->finer->partition_aggregation_weight[i]
                            < weight_threshold);
        }
        assert(!zero);

        graph = graph->finer;
        free(part_count);
        free(part_type);
        free(graph->coarser->partition);
        free(graph->coarser->partition_aggregation_weight);
        free(graph->coarser->map_to_coarser);
        free(graph->coarser->node_aggregation_weight);
        in_memory_file_destroy(graph->coarser->records);
        free(graph->coarser);
    }

    assert(graph->finer == NULL);
    free(graph->map_to_coarser);
    free(graph->partition);
    free(graph->partition_aggregation_weight);
    free(graph->node_aggregation_weight);
    free(graph);
}

void
test_reorder(in_memory_file_t* db)
{
    multi_level_graph_t* graph = malloc(sizeof(*graph));

    if (!graph) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    graph->c_level        = 0;
    graph->finer          = NULL;
    graph->coarser        = NULL;
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(unsigned long));
    graph->node_aggregation_weight =
          calloc(db->node_id_counter, sizeof(unsigned long));

    if (!graph->map_to_coarser || !graph->node_aggregation_weight) {
        printf("Allocating memory failed!\n");
        exit(-1);
    }

    graph->records = db;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        graph->node_aggregation_weight[i] = 1;
    }

    size_t        num_v_matches      = 2;
    float         c_ratio_avg        = 0.0F;
    unsigned long max_partition_size = BLOCK_SIZE / sizeof(node_t);
    list_ul_t**   nodes_per_part;
    bool*         part_type;

    while (coarsen(graph, &num_v_matches, &max_partition_size, &c_ratio_avg)
           == 0) {
        graph = graph->coarser;
    }

    turn_around(graph);

    assert(graph->coarser == NULL);

    while (graph->finer) {
        part_type =
              calloc(graph->finer->records->node_id_counter, sizeof(bool));

        nodes_per_part = calloc(graph->num_partitions, sizeof(list_ul_t*));

        if (!part_type || !nodes_per_part) {
            printf("Allocating memory failed!\n");
            exit(-1);
        }

        for (size_t i = 0; i < graph->num_partitions; ++i) {
            nodes_per_part[i] = create_list_ul();
        }

        for (size_t i = 0; i < graph->finer->records->node_id_counter; ++i) {
            list_ul_append(
                  nodes_per_part
                        [graph->partition[graph->finer->map_to_coarser[i]]],
                  i);
        }

        project(graph, &part_type, c_ratio_avg, nodes_per_part);

        for (size_t i = 0; i < graph->num_partitions; ++i) {
            list_ul_destroy(nodes_per_part[i]);
        }
        free(nodes_per_part);

        reorder(graph, part_type);

        graph = graph->finer;
        free(part_type);
        free(graph->coarser->partition);
        free(graph->coarser->partition_aggregation_weight);
        free(graph->coarser->map_to_coarser);
        free(graph->coarser->node_aggregation_weight);
        in_memory_file_destroy(graph->coarser->records);
        free(graph->coarser);
    }

    assert(graph->finer == NULL);
    free(graph->map_to_coarser);
    free(graph->partition);
    free(graph->partition_aggregation_weight);
    free(graph->node_aggregation_weight);
    free(graph);
}

void
test_full_run(in_memory_file_t* db)
{
    printf("Start applying the G-Store multilevel partitioning algorithm.\n");
    unsigned long* partition = g_store_layout(db);
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

    test_coarsen(db);

    test_turn_arround(db);

    test_project(db);

    test_reorder(db);

    test_full_run(db);

    in_memory_file_destroy(db);
    return 0;
}
