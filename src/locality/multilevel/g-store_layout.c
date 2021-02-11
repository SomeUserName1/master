#include "g-store_layout.h"

#include <stdlib.h>
#include <float.h>

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

int coarsen(multi_level_graph_t* graph, size_t block_size, size_t* num_v_matches, size_t* max_partition_size) {
    float c_ratio = 1.0;
    size_t num_nodes = graph->records->node_id_counter;
    bool node_matched[num_nodes];
    for (size_t i = 0; i < num_nodes; ++i) { node_matched[i] = false; }
    list_relationship_t* rels;
    relationship_t* rel;
    relationship_t* new_rel;
    unsigned long other_node_id;
    unsigned long new_rel_id;
    size_t num_rels;
    // If we want to match n nodes together we need n-1 edges and other nodes as one is fixed by the iteration
    size_t matches[*num_v_matches - 1];
    size_t matches_weights[*num_v_matches - 1];
    for (size_t i = 0; i < *num_v_matches - 1; ++i) { matches[i] = 0; }
    for (size_t i = 0; i < *num_v_matches - 1; ++i) { matches_weights[i] = 0; }
    size_t num_matched = 0;

    multi_level_graph_t* coarser = (multi_level_graph_t*) malloc(sizeof(*coarser));
    coarser->c_level = graph->c_level + 1;
    coarser->node_aggregation_weight = calloc(graph->records->node_id_counter, sizeof(size_t));
    coarser->edge_aggregation_weight = calloc(graph->records->rel_id_counter, sizeof(size_t));
    coarser->map_to_coarser = malloc(num_nodes * sizeof(size_t));
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
        new_rel = in_memory_contains_relationship_from_to(coarser->records,
                graph->map_to_coarser[rel->source_node], graph->map_to_coarser[rel->target_node], BOTH);
        if (new_rel == NULL) {
            new_rel_id = in_memory_create_relationship(coarser->records, graph->map_to_coarser[i],
                    graph->map_to_coarser[other_node_id]);
        } else {
            new_rel_id = new_rel->id;
        }

        coarser->edge_aggregation_weight[new_rel_id] += 1;
    }

    c_ratio = (float) graph->records->node_id_counter / (float) coarser->records->node_id_counter;

    if (c_ratio == 0.0) {
        free(coarser->node_aggregation_weight);
        free(coarser->edge_aggregation_weight);
        free(coarser);
        graph->coarser = NULL;
        return -1;
    } else if (c_ratio < 0.3) {
        if (*num_v_matches == 2 && *max_partition_size <= 32 * block_size) {
            (*max_partition_size) *= 2;
        } else {
            (*num_v_matches)++;
        }
    }

    coarser->node_aggregation_weight = realloc(coarser->node_aggregation_weight, coarser->records->node_id_counter * sizeof(size_t));
    coarser->edge_aggregation_weight = realloc(coarser->node_aggregation_weight, coarser->records->rel_id_counter * sizeof(size_t));

    return 0;
}


void g_store_layout(in_memory_file_t* db, size_t block_size) {
    multi_level_graph_t* graph = malloc(sizeof(*graph));
    graph->records = db;
    graph->c_level = 0;
    graph->node_aggregation_weight = malloc(db->node_id_counter * sizeof(size_t));
    graph->edge_aggregation_weight = malloc(db->rel_id_counter * sizeof(size_t));
    for (size_t i = 0; i < db->node_id_counter; ++i) { graph->node_aggregation_weight[i] = 1; }
    graph->map_to_coarser = calloc(db->node_id_counter, sizeof(size_t));
    graph->finer = NULL;

    size_t num_v_matches = 2;
    size_t max_partition_size = block_size / sizeof(node_t);


    while (coarsen(graph, block_size, &num_v_matches, &max_partition_size) == 0) {
        graph = graph->coarser;
    }

    turn_around(graph);

    while (uncoarsen(graph) == 0) {
        graph = graph->finer;
    }
}

