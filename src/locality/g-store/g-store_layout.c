#include "g-store_layout.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../data-struct/fibonacci_heap.h"
#include "../../data-struct/list_ul.h"

static void
insert_match(size_t*       matches,
             size_t*       match_weights,
             unsigned long node_id,
             double        weight,
             size_t        num_v_matches)
{
    if (!matches || !match_weights) {
        printf("G-Store - insert_match: Invalid Arguments!\n");
        exit(-1);
    }

    bool          placed = false;
    double        temp_w;
    double        temp1_w;
    unsigned long temp_id;
    unsigned long temp1_id;

    for (size_t i = 0; i < num_v_matches; i++) {
        if (placed) {
            temp1_id         = matches[i];
            temp1_w          = match_weights[i];
            matches[i]       = temp_id;
            match_weights[i] = temp_w;
            temp_id          = temp1_id;
            temp_w           = temp1_w;
        } else if (weight > match_weights[i]) {
            temp_id          = matches[i];
            temp_w           = match_weights[i];
            matches[i]       = node_id;
            match_weights[i] = weight;
            placed           = true;
        }
    }
}

unsigned long
compute_abs_tension(multi_level_graph_t* graph,
                    const unsigned long* partition_p_node)
{
    if (!graph || !partition_p_node) {
        printf("G-Store - compute_abs_tension: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long        tension = 0UL;
    long                 diff;
    list_relationship_t* rels = in_memory_get_relationships(graph->records);
    relationship_t*      rel;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);

        diff = ((long)partition_p_node[rel->target_node]
                - (long)partition_p_node[rel->source_node]);

        tension += (unsigned long)fabs(rel->weight * (double)diff);
    }

    list_relationship_destroy(rels);

    return tension;
}

unsigned long
compute_conn_parts(multi_level_graph_t* graph, const unsigned long* partition)
{
    if (!graph || !partition) {
        printf("G-Store - compute_conn_parts: Invalid Arguments!\n");
        exit(-1);
    }

    list_relationship_t* rels = in_memory_get_relationships(graph->records);
    relationship_t*      rel;
    long                 conn_parts = 0;

    for (size_t i = 0; i < graph->num_partitions; ++i) {
        for (size_t j = 0; j < i; ++j) {
            for (size_t i = 0; i < list_relationship_size(rels); ++i) {
                rel = list_relationship_get(rels, i);
                if (partition[rel->source_node] != partition[rel->target_node]
                    && (partition[rel->source_node] == i
                        || partition[rel->source_node] == j)
                    && (partition[rel->target_node] == i
                        || partition[rel->target_node] == j)) {
                    conn_parts++;
                    break;
                }
            }
        }
    }

    list_relationship_destroy(rels);

    return conn_parts;
}

long
compute_num_e_btw_parts(multi_level_graph_t* graph,
                        const unsigned long* partition)
{
    if (!graph || !partition) {
        printf("G-Store - compute_num_e_btw_parts: Invalid Arguments!\n");
        exit(-1);
    }

    list_relationship_t* rels = in_memory_get_relationships(graph->records);
    relationship_t*      rel;
    long                 total_e = 0;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        if (partition[rel->source_node] != partition[rel->target_node]) {
            total_e++;
        }
    }

    list_relationship_destroy(rels);

    return total_e;
}

long*
compute_tension(multi_level_graph_t* graph,
                list_ul_t*           nodes_in_p,
                bool                 modified_t)
{
    if (!graph || !nodes_in_p || !graph->finer || !graph->partition) {
        printf("G-Store - compute_tension: Invalid Arguments!\n");
        exit(-1);
    }

    size_t num_nodes_p = list_ul_size(nodes_in_p);
    long*  tensions    = calloc(num_nodes_p, sizeof(long));

    if (!tensions) {
        printf("G-Store - compute_tension: Memory Allocation failed!\n");
        exit(-1);
    }

    unsigned long        node_id;
    unsigned long        other_node_id;
    long                 p1;
    long                 p2;
    multi_level_graph_t* finer = graph->finer;
    list_relationship_t* rels;
    relationship_t*      rel;

    if (!finer->partition || !finer->map_to_coarser || !finer->records) {
        printf("G-Store - compute_tension: Memory Allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_nodes_p; ++i) {
        node_id = list_ul_get(nodes_in_p, i);

        p1 = modified_t ? (long)graph->partition[finer->map_to_coarser[node_id]]
                        : (long)finer->partition[node_id];

        rels = in_memory_expand(finer->records, node_id, BOTH);

        for (size_t j = 0; j < list_relationship_size(rels); ++j) {
            rel           = list_relationship_get(rels, j);
            other_node_id = rel->source_node == node_id ? rel->target_node
                                                        : rel->source_node;

            p2 = modified_t
                       ? (long)graph
                               ->partition[finer->map_to_coarser[other_node_id]]
                       : (long)finer->partition[other_node_id];

            tensions[i] += (long)(rel->weight * (double)(p2 - p1));
        }
        list_relationship_destroy(rels);
    }
    return tensions;
}

int
compare_by_tension(const void* a, const void* b, void* array2)
{
    if (!a || !b || !array2) {
        printf("G-Store - compare_by_tension: Invalid Arguments!\n");
        exit(-1);
    }

    long diff =
          ((long*)array2)[*(unsigned long*)a] > ((long*)array2)[*(unsigned*)b];

    return (0 < diff) - (diff < 0);
}

unsigned long*
sort_by_tension(list_ul_t* nodes, long* tensions, unsigned long size)
{
    if (!nodes || !tensions) {
        printf("G-Store - sort_by_tension: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long  i;
    unsigned long* s_nodes = calloc(size, sizeof(unsigned long));

    if (!s_nodes) {
        printf("G-Store - sort_by_tension: Memory Allocation failed!\n");
        exit(-1);
    }

    for (i = 0; i < size; i++) {
        s_nodes[i] = list_ul_get(nodes, i);
    }

    qsort_r(nodes, size, sizeof(long), compare_by_tension, (void*)tensions);

    return s_nodes;
}

unsigned long*
swap_partitions(multi_level_graph_t* graph, size_t idx1, size_t idx2)
{
    if (!graph) {
        printf("G-Store - swap_partitions: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long* new_partition =
          calloc(graph->records->node_id_counter, sizeof(unsigned long));

    if (!new_partition) {
        printf("G-Store - swap_partitions: Memory Allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < graph->records->node_id_counter; ++i) {
        if (graph->partition[i] == idx1) {
            new_partition[i] = idx2;
        } else if (graph->partition[i] == idx2) {
            new_partition[i] = idx1;
        } else {
            new_partition[i] = graph->partition[i];
        }
    }

    return new_partition;
}

int
coarsen(multi_level_graph_t* graph,
        size_t               block_size,
        size_t*              num_v_matches,
        size_t*              max_partition_size,
        float*               c_ratio_avg)
{
    if (!graph || !num_v_matches || !max_partition_size || !c_ratio_avg) {
        printf("G-Store - coarsen: Invalid Arguments!\n");
        exit(-1);
    }

    float                c_ratio      = 1.0F;
    size_t               num_nodes    = graph->records->node_id_counter;
    bool*                node_matched = calloc(num_nodes, sizeof(bool));
    list_relationship_t* rels;
    relationship_t*      rel;
    relationship_t*      new_rel;
    unsigned long        other_node_id;
    unsigned long        new_rel_id;
    size_t               num_rels;
    // If we want to match n nodes together we need n-1 edges and other nodes
    size_t* matches         = calloc(*num_v_matches - 1, sizeof(size_t));
    size_t* matches_weights = calloc(*num_v_matches - 1, sizeof(size_t));
    size_t  num_matched     = 0;

    multi_level_graph_t* coarser = calloc(1, sizeof(*coarser));

    if (!coarser || !node_matched || !matches || !matches_weights) {
        printf("G-Store - coarsen: Memory Allocation failed!\n");
        exit(-1);
    }

    coarser->records = create_in_memory_file();

    coarser->c_level = graph->c_level + 1;

    coarser->node_aggregation_weight =
          calloc(graph->records->node_id_counter, sizeof(unsigned long));

    coarser->map_to_coarser = calloc(num_nodes, sizeof(unsigned long));

    if (!coarser->node_aggregation_weight || !coarser->map_to_coarser) {
        printf("G-Store - coarsen: Memory Allocation failed!\n");
        exit(-1);
    }

    coarser->coarser = NULL;
    coarser->finer   = graph;
    graph->coarser   = coarser;

    // group vertices
    for (size_t i = 0; i < num_nodes; ++i) {

        if (node_matched[i]
            || graph->node_aggregation_weight[i] >= *max_partition_size) {
            continue;
        }
        rels = in_memory_expand(graph->records, i, BOTH);

        num_rels = list_relationship_size(rels);

        for (size_t j = 0; j < num_rels; ++j) {
            rel = list_relationship_get(rels, j);
            other_node_id =
                  i == rel->source_node ? rel->target_node : rel->source_node;

            if (node_matched[other_node_id]
                || graph->node_aggregation_weight[other_node_id]
                               + graph->node_aggregation_weight[i]
                         >= *max_partition_size) {
                continue;
            }
            // Smallest weight is stored at last position of the array
            if (rel->weight > matches_weights[*num_v_matches - 2]) {

                insert_match(matches,
                             matches_weights,
                             other_node_id,
                             rel->weight,
                             *num_v_matches - 1);
                num_matched++;
            }
        }

        node_matched[i] = true;
        in_memory_create_node(coarser->records);

        graph->map_to_coarser[i] = coarser->records->node_id_counter - 1;

        coarser->node_aggregation_weight[coarser->records->node_id_counter
                                         - 1] +=
              graph->node_aggregation_weight[i];

        num_matched = num_matched > *num_v_matches - 1 ? *num_v_matches - 1
                                                       : num_matched;

        for (size_t j = 0; j < num_matched; ++j) {
            node_matched[matches[j]] = true;
            coarser->node_aggregation_weight[coarser->records->node_id_counter
                                             - 1] +=
                  graph->node_aggregation_weight[matches[j]];

            graph->map_to_coarser[matches[j]] =
                  coarser->records->node_id_counter - 1;
        }

        num_matched = 0;
        for (size_t i = 0; i < *num_v_matches - 1; ++i) {
            matches[i] = 0;
        }
        for (size_t i = 0; i < *num_v_matches - 1; ++i) {
            matches_weights[i] = 0;
        }
        list_relationship_destroy(rels);
    }

    // group edges
    rels     = in_memory_get_relationships(graph->records);
    num_rels = list_relationship_size(rels);
    for (unsigned long i = 0; i < num_rels; ++i) {
        rel = list_relationship_get(rels, i);

        if (graph->map_to_coarser[rel->source_node]
            != graph->map_to_coarser[rel->target_node]) {

            new_rel = in_memory_contains_relationship_from_to(
                  coarser->records,
                  graph->map_to_coarser[rel->source_node],
                  graph->map_to_coarser[rel->target_node],
                  BOTH);

            if (!new_rel) {
                new_rel_id = in_memory_create_relationship(
                      coarser->records,
                      graph->map_to_coarser[rel->source_node],
                      graph->map_to_coarser[rel->target_node]);
                new_rel =
                      in_memory_get_relationship(coarser->records, new_rel_id);
            }
            new_rel->weight += rel->weight;
        }
    }
    list_relationship_destroy(rels);

    c_ratio = (1.0F
               - ((float)coarser->records->node_id_counter
                  / (float)graph->records->node_id_counter));

    if (c_ratio == 0.0) {
        free(coarser->node_aggregation_weight);
        free(coarser->map_to_coarser);
        in_memory_file_destroy(coarser->records);
        free(coarser);
        free(node_matched);
        free(matches);
        free(matches_weights);
        graph->coarser = NULL;
        return -1;
    }
    if (c_ratio < C_RATIO_LIMIT) {
        if (*num_v_matches == 2
            && *max_partition_size <= MAX_PARTITION_SIZE_FACTOR
                                            * (block_size / sizeof(node_t))) {

            (*max_partition_size) *= 2;
        } else {
            (*num_v_matches)++;
        }
    }

    *c_ratio_avg = (float)((*c_ratio_avg * (float)graph->c_level) + c_ratio)
                   / (float)coarser->c_level;

    coarser->node_aggregation_weight =
          realloc(coarser->node_aggregation_weight,
                  coarser->records->node_id_counter * sizeof(unsigned long));

    if (!coarser->node_aggregation_weight) {
        free(coarser->node_aggregation_weight);
        printf("G-Store - coarsen: Memory Allocation failed!\n");
        exit(-1);
    }

    free(node_matched);
    free(matches);
    free(matches_weights);

    return 0;
}

void
turn_around(multi_level_graph_t* graph, size_t block_size)
{
    if (!graph || !graph->records) {
        printf("G-Store - turn_around: Invalid Arguments!\n");
        exit(-1);
    }

    size_t num_nodes      = graph->records->node_id_counter;
    graph->num_partitions = 0;
    graph->partition      = calloc(num_nodes, sizeof(unsigned long));
    graph->partition_aggregation_weight = calloc(num_nodes, sizeof(size_t));

    if (!graph->partition || !graph->partition_aggregation_weight) {
        printf("G-Store - turn_around: Memory Allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_nodes; ++i) {
        // If partition contains at least one other node
        // and exceeds the block size when added,
        // create a new partition
        if (graph->partition_aggregation_weight[graph->num_partitions] > 0
            && graph->partition_aggregation_weight[graph->num_partitions]
                           + graph->node_aggregation_weight[i]
                     > (block_size / sizeof(node_t))) {

            graph->num_partitions++;
        }
        graph->partition[i] = graph->num_partitions;
        graph->partition_aggregation_weight[graph->num_partitions] +=
              graph->node_aggregation_weight[i];
    }
    // Partition enumeration starts at 0,
    // i.e. the number of partition needs to be incremented
    graph->num_partitions++;
    graph->partition_aggregation_weight =
          realloc(graph->partition_aggregation_weight,
                  graph->num_partitions * sizeof(size_t));

    if (graph->partition_aggregation_weight == NULL) {
        free(graph->partition_aggregation_weight);
        printf("G-Store - turn_around: Memory Allocation failed!\n");
        exit(-1);
    }
}

void
project(multi_level_graph_t* graph,
        bool**               part_type,
        size_t               block_size,
        float                c_ratio_avg,
        list_ul_t**          nodes_per_part)
{
    if (!graph || !graph->finer || !part_type || !*part_type
        || !nodes_per_part) {
        printf("G-Store - project: Invalid Arguments!\n");
        exit(-1);
    }

    multi_level_graph_t* finer       = graph->finer;
    size_t               num_nodes_f = finer->records->node_id_counter;
    finer->num_partitions            = 0;
    finer->partition = calloc(num_nodes_f, sizeof(unsigned long));
    finer->partition_aggregation_weight = calloc(num_nodes_f, sizeof(size_t));

    if (!finer->partition || !finer->partition_aggregation_weight) {
        printf("G-Store - project: Memory Allocation failed!\n");
        exit(-1);
    }

    size_t        finer_partition = 0;
    list_ul_t*    nodes_coarser_p;
    size_t        num_nodes_p;
    size_t        max_finer_p;
    size_t        min_finer_p;
    long*         tensions;
    unsigned long ext_node_id = 0;
    long          ext_tension;
    bool          min;
    float         weight_threshold = ((float)block_size / (float)sizeof(node_t))
                             / (float)pow(1 - c_ratio_avg, finer->c_level);

    printf("num coarser parts: %lu\n", graph->num_partitions);
    for (size_t coarser_part = 0; coarser_part < graph->num_partitions;
         ++coarser_part) {

        nodes_coarser_p = nodes_per_part[coarser_part];
        num_nodes_p     = list_ul_size(nodes_coarser_p);

        printf("pn %lu, pc coarser %lu, paw coarser %lu, wt %.3f\n",
               finer_partition,
               num_nodes_p,
               graph->partition_aggregation_weight[coarser_part],
               weight_threshold);

        if (list_ul_size(nodes_coarser_p) == 1
            || graph->partition_aggregation_weight[coarser_part]
                     < (unsigned long)weight_threshold) {

            for (size_t j = 0; j < num_nodes_p; ++j) {
                finer->partition[list_ul_get(nodes_coarser_p, j)] =
                      finer_partition;

                finer->partition_aggregation_weight[finer_partition] +=
                      finer->node_aggregation_weight[list_ul_get(
                            nodes_coarser_p, j)];
            }

            (*part_type)[finer_partition] = true;
            finer_partition++;

            continue;
        }

        min = true;
        max_finer_p =
              finer_partition
              + ceil((float)graph->partition_aggregation_weight[coarser_part]
                     / weight_threshold)
              - 1;
        min_finer_p = finer_partition;
        do {
            ext_tension = min ? LONG_MAX : LONG_MIN;
            tensions    = compute_tension(graph, nodes_coarser_p, true);

            for (size_t i = 0; i < num_nodes_p; ++i) {
                if ((min && tensions[i] < ext_tension)
                    || (!min && tensions[i] > ext_tension)) {
                    ext_tension = tensions[i];
                    ext_node_id = list_ul_get(nodes_coarser_p, i);
                }
            }
            free(tensions);
            if (min) {
                if ((float)(finer->partition_aggregation_weight[min_finer_p]
                            + finer->node_aggregation_weight[ext_node_id])
                    > weight_threshold) {

                    (*part_type)[min_finer_p] = false;
                    min_finer_p++;
                }

                finer->partition[ext_node_id] = min_finer_p;
                finer->partition_aggregation_weight[min_finer_p] +=
                      finer->node_aggregation_weight[ext_node_id];
            } else {
                if ((float)(finer->partition_aggregation_weight[max_finer_p]
                            + finer->node_aggregation_weight[ext_node_id])
                    > weight_threshold) {

                    (*part_type)[max_finer_p] = true;
                    max_finer_p--;
                }
                finer->partition[ext_node_id] = max_finer_p;
                finer->partition_aggregation_weight[max_finer_p] +=
                      finer->node_aggregation_weight[ext_node_id];
            }
            min = !min;
            list_ul_remove_elem(nodes_coarser_p, ext_node_id);
            num_nodes_p--;
        } while (num_nodes_p > 0);
        finer_partition +=
              ceil((float)graph->partition_aggregation_weight[coarser_part]
                   / weight_threshold);
        (*part_type)[finer_partition - 1] = true;
    }

    finer->num_partitions = finer_partition;
    finer->partition_aggregation_weight =
          realloc(finer->partition_aggregation_weight,
                  finer->num_partitions * sizeof(size_t));

    if (!finer->partition_aggregation_weight) {
        free(finer->partition_aggregation_weight);
        printf("G-Store - project: Memory Allocation failed!\n");
        exit(-1);
    }

    *part_type = realloc(*part_type, finer->num_partitions * sizeof(bool));

    if (!part_type || !*part_type) {
        free(*part_type);
        printf("G-Store - project: Memory Allocation failed!\n");
        exit(-1);
    }
}

void
reorder(multi_level_graph_t* graph, const bool* part_type)
{
    if (!graph || !part_type || !graph->finer) {
        printf("G-Store - reorder: Invalid Arguments!\n");
        exit(-1);
    }

    multi_level_graph_t* finer = graph->finer;
    list_ul_t** groups = calloc(finer->num_partitions, sizeof(list_ul_t*));

    if (!groups) {
        printf("G-Store - reorder: Memory Allocation failed!\n");
        exit(-1);
    }

    size_t num_groups = 0;
    size_t num_parts;
    bool   swapped = false;
    groups[0]      = create_list_ul();
    list_ul_append(groups[0], 0);
    long*          gains;
    long           max_gain;
    unsigned long* swap_p;
    unsigned long* max_gain_p = NULL;
    size_t         max_gain_idx1;
    size_t         max_gain_idx2;
    size_t         temp;

    // extract groups
    for (size_t i = 1; i < finer->num_partitions; ++i) {
        if (part_type[i - 1] == false && part_type[i] == true) {
            num_groups++;
            printf("num groups %lu\n", num_groups);
            groups[num_groups] = create_list_ul();
        }
        printf("Appending: %lu with part type %d to %lu\n",
               i,
               part_type[i],
               num_groups);
        list_ul_append(groups[num_groups], i);
    }
    num_groups++;
    groups = realloc(groups, num_groups * sizeof(list_ul_t*));

    if (!groups) {
        free(groups);
        printf("G-Store - reorder: Memory Allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_groups; ++i) {
        num_parts = list_ul_size(groups[i]);

        if (num_parts == 1) {
            list_ul_destroy(groups[i]);
            continue;
        }

        gains = calloc(num_parts * num_parts, sizeof(long));
        if (!gains) {
            printf("G-Store - reorder: Memory Allocation failed!\n");
            exit(-1);
        }

        do {
            swapped  = false;
            max_gain = LONG_MIN;

            for (size_t j = 0; j < num_parts * num_parts; ++j) {
                gains[j] = LONG_MIN;
            }

            for (size_t j = 0; j < num_parts; ++j) {
                for (size_t k = 0; k < num_parts; ++k) {
                    if (j == k || gains[j * num_parts + k] != LONG_MIN) {
                        continue;
                    }
                    swap_p = swap_partitions(finer, j, k);
                    gains[j * num_parts + k] =
                          (long)(compute_abs_tension(graph, finer->partition)
                                 - compute_abs_tension(graph, swap_p));
                    gains[k * num_parts + j] = gains[j * num_parts + k];
                    if (gains[j * num_parts + k] > max_gain) {
                        max_gain = gains[j * num_parts + k];
                        free(max_gain_p);
                        max_gain_p    = swap_p;
                        max_gain_idx1 = j;
                        max_gain_idx2 = k;
                    } else {
                        free(swap_p);
                    }
                }
            }

            if (max_gain > 0) {
                free(finer->partition);
                finer->partition = max_gain_p;
                temp = finer->partition_aggregation_weight[max_gain_idx1];
                finer->partition_aggregation_weight[max_gain_idx1] =
                      finer->partition_aggregation_weight[max_gain_idx2];
                finer->partition_aggregation_weight[max_gain_idx2] = temp;
                swapped                                            = true;
            }

        } while (swapped);
        free(gains);
        list_ul_destroy(groups[i]);
    }
    free(groups);
}

void
refine(multi_level_graph_t* graph, size_t block_size, float c_ratio_avg)
{
    if (!graph || !graph->finer || !graph->finer->records) {
        printf("G-Store - refine: Invalid Arguments!\n");
        exit(-1);
    }

    multi_level_graph_t* finer     = graph->finer;
    size_t               num_nodes = finer->records->node_id_counter;
    float* score = calloc(num_nodes * finer->num_partitions, sizeof(float));

    float weight_threshold = ((float)block_size / (float)sizeof(node_t))
                             / (float)pow(1 - c_ratio_avg, finer->c_level);

    unsigned long* temp_p = calloc(num_nodes, sizeof(unsigned long));

    if (!score || !temp_p) {
        printf("G-Store - refine: Memory Allocation failed!\n");
        exit(-1);
    }

    float  occupancy_factor = 0;
    float  max_score;
    size_t node_id      = ULONG_MAX;
    size_t partition_id = ULONG_MAX;

    // TODO This should actually be an adapted KL
    for (size_t m = 0; m < REFINEMENT_ITERS; ++m) {
        for (size_t i = 0; i < finer->records->node_id_counter; ++i) {
            for (size_t k = 0; k < finer->num_partitions; ++k) {
                // copy temp partition from original
                // & set current nodes partition according to k
                for (size_t n = 0; n < num_nodes; ++n) {
                    temp_p[n] = finer->partition[n];
                }

                temp_p[i] = k;

                // TODO this should be an additional term in the eq.
                // however what's described in the report is rather clumsy
                // http://g-store.sourceforge.net/th/6.htm#6ddre
                occupancy_factor =
                      1
                      - (float)(finer->node_aggregation_weight[i]
                                + finer->partition_aggregation_weight[k])
                              / weight_threshold;

                score[i * finer->num_partitions + k] =
                      occupancy_factor
                      * (ALPHA * (float)compute_abs_tension(finer, temp_p)
                         + BETA * compute_conn_parts(finer, temp_p)
                         + GAMMA * compute_num_e_btw_parts(finer, temp_p));
            }
        }
        max_score = FLT_MIN;
        for (size_t i = 0; i < num_nodes * finer->num_partitions; ++i) {
            if (score[i] > max_score) {
                score[i]     = max_score;
                node_id      = i / finer->num_partitions;
                partition_id = i % finer->num_partitions;
            }
        }
        if (max_score - score[node_id * finer->num_partitions + partition_id]
            > 0.0F) {
            finer->partition[node_id] = partition_id;
        }
    }
    free(score);
    free(temp_p);
}

int
uncoarsen(multi_level_graph_t* graph, size_t block_size, float c_ratio_avg)
{
    if (graph->finer == NULL) {
        return -1;
    }

    bool* part_type =
          calloc(graph->finer->records->node_id_counter, sizeof(bool));

    if (!part_type) {
        printf("G-Store - uncoarsen: Memory Allocation failed!\n");
        exit(-1);
    }

    list_ul_t** nodes_per_part =
          calloc(graph->num_partitions, sizeof(list_ul_t*));

    for (size_t i = 0; i < graph->num_partitions; ++i) {
        nodes_per_part[i] = create_list_ul();
    }

    for (size_t i = 0; i < graph->finer->records->node_id_counter; ++i) {
        list_ul_append(
              nodes_per_part[graph->partition[graph->finer->map_to_coarser[i]]],
              i);
    }

    project(graph, &part_type, block_size, c_ratio_avg, nodes_per_part);
    for (size_t i = 0; i < graph->num_partitions; ++i) {
        list_ul_destroy(nodes_per_part[i]);
    }
    free(nodes_per_part);

    reorder(graph, part_type);
    free(part_type);

    refine(graph, block_size, c_ratio_avg);

    return 0;
}

unsigned long*
g_store_layout(in_memory_file_t* db, size_t block_size)
{
    if (!db) {
        printf("G-Store - main: Invalid Arguments!\n");
        exit(-1);
    }
    multi_level_graph_t* graph = calloc(1, sizeof(*graph));
    if (!graph) {
        printf("G-Store - main: Memory Allocation failed!\n");
        exit(-1);
    }
    graph->c_level = 0;
    graph->records = db;

    graph->node_aggregation_weight =
          calloc(db->node_id_counter, sizeof(unsigned long));

    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(unsigned long));

    if (!graph->node_aggregation_weight || !graph->map_to_coarser) {
        printf("G-Store - main: Memory Allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        graph->node_aggregation_weight[i] = 1;
    }

    graph->finer = NULL;

    size_t num_v_matches      = 2;
    size_t max_partition_size = block_size / sizeof(node_t);
    float  c_ratio_avg        = 0.0F;

    while (coarsen(graph,
                   block_size,
                   &num_v_matches,
                   &max_partition_size,
                   &c_ratio_avg)
           == 0) {
        graph = graph->coarser;
    }

    turn_around(graph, block_size);

    while (uncoarsen(graph, block_size, c_ratio_avg) == 0) {
        graph = graph->finer;
        free(graph->coarser->map_to_coarser);
        free(graph->coarser->node_aggregation_weight);
        free(graph->coarser->partition_aggregation_weight);
        free(graph->coarser->partition);
        in_memory_file_destroy(graph->coarser->records);
        free(graph->coarser);
    }

    free(graph->map_to_coarser);
    free(graph->node_aggregation_weight);
    free(graph->partition_aggregation_weight);
    unsigned long* result = graph->partition;
    free(graph);

    return result;
}
