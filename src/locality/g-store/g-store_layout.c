#include "g-store_layout.h"

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <limits.h>

#include "../../data-struct/list_ul.h"

void insert_match(size_t* matches, size_t* match_weights, relationship_t* rel, size_t num_v_matches) {
    bool placed = false;
    double temp_w, temp1_w;
    unsigned long temp_rel_id, temp1_rel_id;

    for (size_t i = 0; i < num_v_matches; i++) {
        if (placed) {
            temp1_rel_id = matches[i];
            temp1_w = match_weights[i];
            matches[i] = temp_rel_id;
            match_weights[i] = temp_w;
            temp_rel_id = temp1_rel_id;
            temp_w = temp1_w;
        } else if (rel->weight > match_weights[i]) {
            temp_rel_id = matches[i];
            temp_w = match_weights[i];
            matches[i] = rel->id;
            match_weights[i] = rel->weight;
            placed = true;
        }
    }
}

long compute_abs_tension(multi_level_graph_t* graph, size_t* partition_p_node) {
    unsigned long tension = 0UL;
    long long diff;
    multi_level_graph_t* finer = graph->finer;
    list_relationship_t* rels = in_memory_get_relationships(finer->records);
    relationship_t* rel;

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);

        diff = partition_p_node[rel->target_node] - partition_p_node[rel->source_node];
        tension += finer->edge_aggregation_weight[rel->id] * labs(diff);
    }
    return tension;

}

unsigned long compute_total_e_w_btw_blocks(multi_level_graph_t* graph, size_t* partition) {
    list_relationship_t* rels = in_memory_get_relationships(graph->records);
    relationship_t* rel;
    long total_e_w = 0;

    for (size_t i = 0; 0 < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        if (partition[rel->source_node] != partition[rel->target_node]) {
            total_e_w += graph->edge_aggregation_weight[i];
        }
    }
    return total_e_w;
}

long compute_num_e_btw_blocks(multi_level_graph_t* graph, size_t* partition) {
    list_relationship_t* rels = in_memory_get_relationships(graph->records);
    relationship_t* rel;
    long total_e = 0;

    for (size_t i = 0; 0 < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        if (partition[rel->source_node] != partition[rel->target_node]) {
            total_e++;
        }
    }
    return total_e;
}

long* compute_tension(multi_level_graph_t* graph, list_ul_t* nodes_in_p, bool modified_t) {
    size_t num_nodes_p = list_ul_size(nodes_in_p);
    long* tensions = calloc(num_nodes_p, sizeof(long));
    unsigned long node_id;
    unsigned long other_node_id;
    long p1, p2;
    multi_level_graph_t* finer = graph->finer;
    list_relationship_t* rels;
    relationship_t* rel;

    for (size_t i = 0; i < num_nodes_p; ++i) {
        node_id = list_ul_get(nodes_in_p, i);

        p1 = modified_t ? graph->partition[finer->map_to_coarser[node_id]]
            : finer->partition[node_id];

        rels = in_memory_expand(finer->records, node_id, BOTH);

        for (size_t j = 0; j < list_relationship_size(rels); ++j) {
            rel = list_relationship_get(rels, j);
            other_node_id = rel->source_node == node_id ? rel->target_node : rel->source_node;

            p2 = modified_t ? graph->partition[finer->map_to_coarser[other_node_id]] :
                finer->partition[other_node_id];

            tensions[i] += finer->edge_aggregation_weight[rel->id] * (p2 - p1);
        }
    }
    return tensions;
}

int compare_by_tension(const void *a, const void *b, void *array2) {
    long diff = ((long *)array2)[*(unsigned long *)a]
        > ((long *)array2)[*(unsigned *)b];
    return (0 < diff) - (diff < 0);
}

unsigned long* sort_by_tension(list_ul_t* nodes, long *tensions, unsigned long size) {
    unsigned long i;
    unsigned long *s_nodes = malloc(size * sizeof(unsigned long));

    for (i = 0; i < size; i++) {
        s_nodes[i] = list_ul_get(nodes, i);
    }

    qsort_r(nodes, size, sizeof(long), compare_by_tension, (void *)tensions);

    return s_nodes;
}

size_t* swap_partitions(multi_level_graph_t* graph, size_t idx1, size_t idx2) {
    size_t* new_partition = malloc(graph->records->node_id_counter * sizeof(size_t));

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

int coarsen(multi_level_graph_t* graph, size_t block_size, size_t* num_v_matches, size_t* max_partition_size, float* c_ratio_avg) {
    float c_ratio = 1.0;
    size_t num_nodes = graph->records->node_id_counter;
    bool* node_matched = calloc(num_nodes, sizeof(bool));
    list_relationship_t* rels;
    relationship_t* rel;
    relationship_t* new_rel;
    unsigned long other_node_id;
    unsigned long new_rel_id;
    size_t num_rels;
    // If we want to match n nodes together we need n-1 edges and other nodes as one is fixed by the iteration
    size_t* matches = calloc(*num_v_matches - 1,  sizeof(size_t));
    size_t* matches_weights = calloc(*num_v_matches - 1, sizeof(size_t));
    size_t num_matched = 0;

    multi_level_graph_t* coarser = malloc(sizeof(*coarser));
    coarser->c_level = graph->c_level + 1;
    coarser->node_aggregation_weight = calloc(graph->records->node_id_counter, sizeof(size_t));
    coarser->edge_aggregation_weight = calloc(graph->records->rel_id_counter, sizeof(size_t));
    coarser->map_to_coarser = calloc(num_nodes, sizeof(size_t));
    coarser->coarser = NULL;
    coarser->finer = graph;
    graph->coarser = coarser;

    // group vertices
    for (size_t i = 0; i < num_nodes; ++i) {

        if (node_matched[i]) {
            continue;
        }
        rels = in_memory_expand(graph->records, i, BOTH);

        num_rels = list_relationship_size(rels);

        for (size_t j = 0; j < num_rels; ++j) {
            rel = list_relationship_get(rels, j);
            other_node_id = i == rel->source_node ? rel->target_node : rel->source_node;
            if (node_matched[other_node_id]) {
                continue;
            }
            // Smallest weight is stored at last position of the array
            if (graph->edge_aggregation_weight[rel->id] > matches_weights[*num_v_matches - 2]) {
                insert_match(matches, matches_weights, rel, *num_v_matches - 1);
                num_matched++;
            }
        }

        node_matched[i] = true;
        in_memory_create_node(coarser->records);
        num_matched = num_matched > *num_v_matches ? *num_v_matches : num_matched;

        for (size_t j = 0; j < num_matched; ++j) {
            node_matched[matches[j]] = true;
            coarser->node_aggregation_weight[i] += graph->node_aggregation_weight[matches[j]];
            graph->map_to_coarser[matches[j]] = coarser->records->node_id_counter;
        }

        num_matched = 0;
        for (size_t i = 0; i < *num_v_matches - 1; ++i) { matches[i] = 0; }
        for (size_t i = 0; i < *num_v_matches - 1; ++i) { matches_weights[i] = 0; }
    }
    // group edges
    rels = in_memory_get_relationships(graph->records);
    num_rels = list_relationship_size(rels);
    for (unsigned long i = 0; i < num_rels; ++i) {
        rel = list_relationship_get(rels, i);

        if (graph->map_to_coarser[rel->source_node] != graph->map_to_coarser[rel->target_node]) {
            new_rel = in_memory_contains_relationship_from_to(coarser->records,
                    graph->map_to_coarser[rel->source_node], graph->map_to_coarser[rel->target_node], BOTH);


            if (new_rel == NULL) {
                new_rel_id = in_memory_create_relationship(coarser->records, graph->map_to_coarser[i],
                        graph->map_to_coarser[other_node_id]);
            } else {
                new_rel_id = new_rel->id;
            }

            coarser->edge_aggregation_weight[new_rel_id] += graph->edge_aggregation_weight[rel->id];
        }
    }

    c_ratio = (1.0 - (float) coarser->records->node_id_counter / (float) graph->records->node_id_counter);

    if (c_ratio == 0.0) {
        free(coarser->node_aggregation_weight);
        free(coarser->edge_aggregation_weight);
        free(coarser);
        free(node_matched);
        free(matches);
        free(matches_weights);
        graph->coarser = NULL;
        return -1;
    } else if (c_ratio < 0.3) {
        if (*num_v_matches == 2 && *max_partition_size <= 32 * block_size) {
            (*max_partition_size) *= 2;
        } else {
            (*num_v_matches)++;
        }
    }

    *c_ratio_avg = (float) ((*c_ratio_avg * graph->c_level) + c_ratio) / (float) coarser->c_level;

    coarser->node_aggregation_weight = realloc(coarser->node_aggregation_weight,
            coarser->records->node_id_counter * sizeof(size_t));
    coarser->edge_aggregation_weight = realloc(coarser->node_aggregation_weight,
            coarser->records->rel_id_counter * sizeof(size_t));
    free(node_matched);
    free(matches);
    free(matches_weights);

    return 0;
}

void turn_around(multi_level_graph_t* graph, size_t block_size) {
    size_t num_nodes = graph->records->node_id_counter;
    graph->num_partitions = 0;
    graph->partition = calloc(num_nodes, sizeof(size_t));
    graph->partition_aggregation_weight = calloc(num_nodes, sizeof(size_t));

    for (size_t i = 0; i < num_nodes; ++i) {
        // If partition contains at least one other node and exceeds the block size when added,
        // create a new partition
        if (graph->partition_aggregation_weight[graph->num_partitions] > 0
                && graph->partition_aggregation_weight[graph->num_partitions]
                + graph->node_aggregation_weight[i] > block_size) {
            graph->num_partitions++;
        }
        graph->partition[i] = graph->num_partitions;
        graph->partition_aggregation_weight[graph->num_partitions] += graph->node_aggregation_weight[i];
    }
    // Partition enumeration starts at 0, i.e. the number of partition needs to be incremented
    graph->num_partitions++;
    graph->partition_aggregation_weight = realloc(
            graph->partition_aggregation_weight, graph->num_partitions * sizeof(size_t));
}

void project(multi_level_graph_t* graph, bool* part_type, size_t block_size,
        float c_ratio_avg,  list_ul_t** nodes_per_part) {
    multi_level_graph_t* finer = graph->finer;
    size_t num_nodes_f = finer->records->node_id_counter;
    finer->num_partitions = 0;
    finer->partition = calloc(num_nodes_f, sizeof(size_t));
    finer->partition_aggregation_weight = calloc(num_nodes_f, sizeof(size_t));

    size_t finer_partition = 0;
    list_ul_t* nodes_coarser_p;
    size_t num_nodes_p;
    long* tensions;
    float weight_threshold = block_size / pow(1 - c_ratio_avg, finer->c_level);


    for (size_t coarser_part = 0; coarser_part < graph->num_partitions; ++coarser_part) {
        nodes_coarser_p = nodes_per_part[coarser_part];
        num_nodes_p = list_ul_size(nodes_coarser_p);

        if (list_ul_size(nodes_coarser_p) == 1
                || graph->partition_aggregation_weight[coarser_part] < weight_threshold) {

            for (size_t j = 0; j < num_nodes_p; ++j) {
                finer->partition[list_ul_get(nodes_coarser_p, j)] = finer_partition;
                finer->partition_aggregation_weight[finer_partition]
                    += finer->node_aggregation_weight[list_ul_get(nodes_coarser_p, j)];
            }

            part_type[finer_partition] = true;
            finer_partition++;

            continue;
        }
        // Compute tensions
        tensions = compute_tension(graph, nodes_coarser_p, true);
        // sort & group by tension
        unsigned long* sorted_nodes_p = sort_by_tension(nodes_coarser_p, tensions, num_nodes_p);
        free(tensions);

        for (size_t i = 0; i < num_nodes_p; ++i) {
            if (finer->partition_aggregation_weight[graph->num_partitions] > 0
                    && finer->partition_aggregation_weight[graph->num_partitions]
                    + finer->node_aggregation_weight[sorted_nodes_p[i]] > weight_threshold) {
                part_type[finer_partition] = i < num_nodes_p / 2 + 1 ? false : true;
                finer_partition++;
            }
            finer->partition[sorted_nodes_p[i]] = finer_partition;
            finer->partition_aggregation_weight[finer_partition]
                += finer->node_aggregation_weight[sorted_nodes_p[i]];
        }
        free(sorted_nodes_p);
    }
    finer_partition++;
    finer->num_partitions = finer_partition;
    finer->partition_aggregation_weight = realloc(
            finer->partition_aggregation_weight, finer->num_partitions * sizeof(size_t));
    part_type = realloc(part_type, finer->num_partitions * sizeof(bool));
}

void reorder(multi_level_graph_t* graph, bool* part_type) {
    multi_level_graph_t* finer = graph->finer;
    list_ul_t** groups = calloc(finer->num_partitions, sizeof(list_ul_t*));
    size_t num_groups = 0;
    size_t num_parts;
    bool swapped = false;
    groups[0] = create_list_ul(LIST_NONE);
    list_ul_append(groups[0], 0);
    long* gains;
    long max_gain;
    size_t* swap_p;
    size_t* max_gain_p;
    size_t max_gain_idx1, max_gain_idx2, temp;
    list_ul_t* temp_npp;


    // extract groups
    for (size_t i = 1; i < finer->num_partitions; ++i) {
        if (part_type[i - 1] == false && part_type[i] == true) {
            num_groups++;
            groups[num_groups] = create_list_ul(LIST_NONE);
        }
        list_ul_append(groups[num_groups], i);
    }
    groups = realloc(groups, num_groups * sizeof(list_t*));

    for (size_t i = 0; i < num_groups; ++i) {
        num_parts = list_ul_size(groups[i]);
        if (num_parts == 1) {
            continue;
        }
        gains = malloc(num_parts * num_parts * sizeof(long));
        do {
            swapped = false;
            max_gain = LONG_MIN;
            for (size_t j = 0; j < num_parts * num_parts; ++j) { gains[j] = LONG_MIN; }

            for (size_t j = 0; j < num_parts; ++j) {
                for (size_t k = 0; k < num_parts; ++k) {
                    if (j == k || gains[j * num_parts + k] != LONG_MIN) {
                        continue;
                    }
                    swap_p = swap_partitions(finer, j, k);
                    gains[j * num_parts + k] = compute_abs_tension(graph, finer->partition) - compute_abs_tension(graph, swap_p);
                    gains[k * num_parts + j] = gains[j * num_parts + k];
                    if (gains[j * num_parts + k] > max_gain) {
                        max_gain = gains[j * num_parts + k];
                        free(max_gain_p);
                        max_gain_p = swap_p;
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
                finer->partition_aggregation_weight[max_gain_idx1] = finer->partition_aggregation_weight[max_gain_idx2];
                finer->partition_aggregation_weight[max_gain_idx2] = temp;
                swapped = true;
                free(swap_p);
            }
        } while (swapped);
        free(gains);
        list_ul_destroy(groups[i]);
    }
    free(groups);
}

void refine(multi_level_graph_t* graph, size_t block_size, float c_ratio_avg) {
    multi_level_graph_t* finer;
    size_t num_nodes = finer->records->node_id_counter;
    float* score = calloc(num_nodes * finer->num_partitions, sizeof(float));
    float weight_threshold = block_size / pow(1 - c_ratio_avg, finer->c_level);
    size_t* temp_p = calloc(num_nodes, sizeof(size_t));
    long occupancy_factor = 0;
    float max_score;
    size_t node_id, partition_id;

    for (size_t m = 0; m < REFINEMENT_ITERS; ++m) {
        for (size_t i = 0; i < finer->records->node_id_counter; ++i) {
            for (size_t k = 0; k < finer->num_partitions; ++k) {
                if (k == finer->partition[i]) { continue; }
                // copy temp partition from original & set current nodes partition according to k
                for (size_t n = 0; n < num_nodes; ++n) {
                    temp_p[n] = finer->partition[n];
                }
                temp_p[i] = k;
                occupancy_factor = 1 - (finer->node_aggregation_weight[i] + finer->partition_aggregation_weight[k]) / weight_threshold;
                score[i * finer->num_partitions + k] 
                    = - (ALPHA * compute_abs_tension(finer, temp_p)
                    + BETA * compute_total_e_w_btw_blocks(graph, temp_p)
                    + GAMMA * compute_num_e_btw_blocks(graph, temp_p));
            }
        }
        max_score = 0.0f;
        for (size_t i = 0; i < num_nodes * finer->num_partitions; ++i) {
            if (score[i] > max_score) {
                node_id = i / finer->num_partitions;
                partition_id = i % finer->num_partitions;
            }
        }
        finer->partition[node_id] = partition_id;
    }
}

int uncoarsen(multi_level_graph_t* graph, size_t block_size, float c_ratio_avg) {
    if (graph->finer == NULL) {
        return -1;
    }

    bool* part_type = calloc(graph->finer->records->node_id_counter, sizeof(bool));
    list_ul_t* nodes_per_part[graph->num_partitions];
    for (size_t i = 0; i < graph->num_partitions; ++i) { nodes_per_part[i] = create_list_ul(LIST_NONE); }

    for (size_t i = 0; i < graph->finer->records->node_id_counter; ++i) {
        list_ul_append(nodes_per_part[graph->partition[graph->finer->map_to_coarser[i]]], i);
    }

    project(graph, part_type, block_size, c_ratio_avg, nodes_per_part);
    for (size_t i = 0; i < graph->num_partitions; ++i) { list_ul_destroy(nodes_per_part[i]); }

    reorder(graph, part_type);
    free(part_type);

    refine(graph, block_size, c_ratio_avg);


    return 0;
}

void g_store_layout(in_memory_file_t* db, size_t block_size) {
    multi_level_graph_t* graph = malloc(sizeof(*graph));
    graph->c_level = 0;
    graph->records = db;
    graph->node_aggregation_weight = malloc(db->node_id_counter * sizeof(size_t));
    graph->edge_aggregation_weight = malloc(db->rel_id_counter * sizeof(size_t));
    for (size_t i = 0; i < db->node_id_counter; ++i) { graph->node_aggregation_weight[i] = 1; }
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(size_t));
    graph->finer = NULL;

    size_t num_v_matches = 2;
    size_t max_partition_size = block_size / sizeof(node_t);
    float c_ratio_avg = 0.0;

    while (coarsen(graph, block_size, &num_v_matches, &max_partition_size, &c_ratio_avg) == 0) {
        graph = graph->coarser;
    }

    turn_around(graph, block_size);

    while (uncoarsen(graph, block_size, c_ratio_avg) == 0) {
        graph = graph->finer;
    }

    // TODO Write result to file or sth.
}
