#include "icbl.h"

#define _GNU_SOURCE

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../constants.h"
#include "../../data-struct/list_ul.h"
#include "../../query/degree.h"
#include "../../query/random_walk.h"

#define SHARE_OF_MEMORY (0.8)

static size_t
get_num_walks(in_memory_file_t* db)
{
    size_t min_deg = get_min_degree(db, BOTH);
    size_t max_deg = get_max_degree(db, BOTH);
    size_t range = max_deg - min_deg + 1;
    size_t nodes_per_step = db->node_id_counter / range;

    list_relationship_t* rels;

    size_t* degree_hist = calloc(range, sizeof(size_t));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);
        degree_hist[min_deg - list_relationship_size(rels)]++;
        list_relationship_destroy(rels);
    }

    for (size_t i = 0; i < range; ++i) {
        if (degree_hist[i] > nodes_per_step) {
            return i;
        }
    }
    free(degree_hist);

    return max_deg;
}

static inline size_t
get_num_coarse_clusters(in_memory_file_t* db)
{
    return ceil(((double)((sizeof(node_t) + log(db->node_id_counter)) * db->node_id_counter)) /
                sqrt(SHARE_OF_MEMORY * MEMORY));
}

static inline size_t
get_num_steps(in_memory_file_t* db)
{
    return 1 +
           ceil(log2((float)db->node_id_counter) / get_num_coarse_clusters(db));
}

int
identify_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets)
{
    size_t num_nodes = db->node_id_counter;
    size_t num_walks = get_num_walks(db);
    size_t num_steps = get_num_steps(db);
    path_t* result;
    unsigned long node_id;

    for (size_t i = 0; i < num_nodes; ++i) {
        for (size_t j = 0; j < num_walks; ++j) {
            result = random_walk(db, i, num_steps, BOTH);

            for (size_t k = 0; k < list_ul_size(result->visited_nodes); ++k) {
                node_id = list_ul_get(result->visited_nodes, k);
                if (dict_ul_ul_contains(dif_sets[i], node_id)) {
                    dict_ul_ul_insert(
                          dif_sets[i],
                          node_id,
                          dict_ul_ul_get_direct(dif_sets[i], node_id) + 1);
                } else {
                    dict_ul_ul_insert(dif_sets[i], node_id, 1);
                }
            }
        }
    }
    return 0;
}

static float
weighted_jaccard_dist(dict_ul_ul_t* dif_set_a, dict_ul_ul_t* dif_set_b)
{
    dict_ul_ul_iterator_t* it = create_dict_ul_ul_iterator(dif_set_a);
    list_ul_t* visited_elems_b = create_list_ul();

    unsigned long key = 0;
    unsigned long value_a = 0;
    unsigned long value_b = 0;

    unsigned long intersect_sum = 0;
    unsigned long union_sum = 0;

    while (dict_ul_ul_iterator_next(it, &key, &value_a) > -1) {
        if (dict_ul_ul_contains(dif_set_b, key)) {
            list_ul_append(visited_elems_b, key);
            value_b = dict_ul_ul_get_direct(dif_set_b, key);

            intersect_sum += value_a > value_b ? value_b : value_a;
            union_sum += value_a > value_b ? value_a : value_b;
        } else {
            union_sum += value_a;
        }
    }
    dict_ul_ul_iterator_destroy(it);
    it = create_dict_ul_ul_iterator(dif_set_b);

    while (dict_ul_ul_iterator_next(it, &key, &value_b) > -1) {
        if (!list_ul_contains(visited_elems_b, key)) {
            union_sum += value_b;
        }
    }

    dict_ul_ul_iterator_destroy(it);
    list_ul_destroy(visited_elems_b);

    return 1 - ((float)intersect_sum / (float)union_sum);
}

static void
insert_match(size_t* max_degree_nodes,
             size_t* max_degrees,
             unsigned long node_id,
             unsigned long degree,
             size_t num_clusters)
{
    bool placed = false;
    unsigned long temp_d;
    unsigned long temp1_d;
    unsigned long temp_id;
    unsigned long temp1_id;

    for (size_t i = 0; i < num_clusters; i++) {
        if (placed) {
            temp1_id = max_degree_nodes[i];
            temp1_d = max_degrees[i];
            max_degree_nodes[i] = temp_id;
            max_degrees[i] = temp_d;
            temp_id = temp1_id;
            temp_d = temp1_d;
        } else if (degree > max_degrees[i]) {
            temp_id = max_degree_nodes[i];
            temp_d = max_degrees[i];
            max_degree_nodes[i] = node_id;
            max_degrees[i] = degree;
            placed = true;
        }
    }
}

int
initialize_centers(in_memory_file_t* db,
                   unsigned long** centers,
                   size_t num_clusters)
{
    if (!db || !centers) {
        exit(-1);
    }

    unsigned long num_nodes = db->node_id_counter;
    unsigned long* max_degree_nodes =
          calloc(num_clusters, sizeof(unsigned long));
    unsigned long* max_degrees = calloc(num_clusters, sizeof(unsigned long));
    list_relationship_t* rels;
    unsigned long degree;

    if (!max_degrees || !max_degree_nodes) {
        exit(-1);
    }
    // FIXME check distance first
    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, BOTH);
        degree = list_relationship_size(rels);
        if (degree > max_degrees[num_clusters - 1]) {
            insert_match(
                  max_degree_nodes, max_degrees, i, num_clusters, degree);
        }
    }
    list_relationship_destroy(rels);
    free(max_degrees);

    centers = &max_degree_nodes;
    return 0;
}

size_t
assign_to_cluster(size_t num_nodes,
                  dict_ul_ul_t** dif_sets,
                  unsigned long* part,
                  const unsigned long* centers,
                  size_t num_clusters)
{
    float min_dist = FLT_MAX;
    float dist;
    unsigned long best_center_idx = ULONG_MAX;

    for (size_t i = 0; i < num_nodes; ++i) {
        for (size_t j = 0; j < num_clusters; ++j) {
            dist = weighted_jaccard_dist(dif_sets[centers[j]], dif_sets[i]);
            if (dist < min_dist) {
                best_center_idx = j;
            }
        }
        part[i] = best_center_idx;

        min_dist = 0;
        best_center_idx = ULONG_MAX;
    }

    return 0;
}

int
updated_centers(size_t num_nodes,
                dict_ul_ul_t** dif_sets,
                const unsigned long* part,
                unsigned long* centers,
                size_t num_clusters)
{
    if (!dif_sets || !part || !centers) {
        exit(-1);
    }

    dict_ul_ul_t** cluster_counts =
          malloc(num_clusters * sizeof(dict_ul_ul_t*));

    if (!cluster_counts) {
        return -1;
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        cluster_counts[i] = create_dict_ul_ul();
    }

    dict_ul_ul_iterator_t* it;
    unsigned long node_id;
    unsigned long count;

    unsigned long max_node_id = UNINITIALIZED_LONG;
    unsigned int max_count = 0;

    for (size_t i = 0; i < num_nodes; ++i) {
        it = create_dict_ul_ul_iterator(dif_sets[i]);
        while (dict_ul_ul_iterator_next(it, &node_id, &count) > -1) {
            if (dict_ul_ul_contains(cluster_counts[part[i]], node_id)) {
                dict_ul_ul_insert(
                      cluster_counts[part[i]],
                      node_id,
                      dict_ul_ul_get_direct(cluster_counts[part[i]], node_id) +
                            count);
            } else {
                dict_ul_ul_insert(dif_sets[part[i]], node_id, count);
            }
        }
        dict_ul_ul_iterator_destroy(it);
    }
    
    for (size_t i = 0; i < num_clusters; ++i) {
        it = create_dict_ul_ul_iterator(cluster_counts[i]);
        while (dict_ul_ul_iterator_next(it, &node_id, &count) > -1) {
            if (count > max_count) {
                max_node_id = node_id;
            }
        }
        centers[i] = max_node_id;
        max_count = 0;
        max_node_id = UNINITIALIZED_LONG;
        dict_ul_ul_iterator_destroy(it);
    }

    free(cluster_counts);

    return 0;
}

int
cluster_coarse(in_memory_file_t* db,
               dict_ul_ul_t** dif_sets,
               unsigned long* part)
{
    if (!db || !dif_sets || !part) {
        exit(-1);
    }

    size_t num_clusters = get_num_coarse_clusters(db);
    size_t changes = 0;

    unsigned long* centers = NULL;

    if (initialize_centers(db, &centers, num_clusters) < 0) {
        return -1;
    }

    changes = assign_to_cluster(
          db->node_id_counter, dif_sets, part, centers, num_clusters);

    do {
        updated_centers(
              db->node_id_counter, dif_sets, part, centers, num_clusters);
        changes = assign_to_cluster(
              db->node_id_counter, dif_sets, part, centers, num_clusters);
    } while (changes > 0);

    free(centers);

    return 0;
}

int
find_min_dist(const float* pairwise_diff,
              size_t n_nodes,
              unsigned long* idx,
              dendrogram_t** dendros)
{
    float min_dist = FLT_MAX;

    // Compute pairwise distance matrix
    for (size_t i = 0; i < n_nodes; ++i) {
        for (size_t j = 0; j < n_nodes - i; ++j) {
            if (i == j || dendros[i] == dendros[j]) {
                continue;
            }
            if (pairwise_diff[i * n_nodes + j] < min_dist) {
                min_dist = pairwise_diff[i * n_nodes + j];
                idx[0] = i;
                idx[1] = j;
            }
        }
    }
    return 0;
}

int
assign_dendro_label(dendrogram_t* dendro,
                    dendrogram_t* fst_child,
                    dendrogram_t* snd_child)
{
    if (fst_child->size > BLOCK_SIZE && snd_child->size > BLOCK_SIZE) {
        // block label of first + : + block label of snd + \0
        dendro->label = malloc(
              (strlen(fst_child->label) + 1 + strlen(snd_child->label) + 1) *
              sizeof(char));
        if (dendro->label == NULL) {
            return -1;
        }
        dendro->label[0] = '\0';

        if (fst_child->size > snd_child->size) {
            strncat(dendro->label, fst_child->label, strlen(fst_child->label));
            strcat(dendro->label, ":");
            strncat(dendro->label, snd_child->label, strlen(snd_child->label));
        } else {
            strncat(dendro->label, snd_child->label, strlen(snd_child->label));
            strcat(dendro->label, ":");
            strncat(dendro->label, fst_child->label, strlen(fst_child->label));
        }
    } else {
        char* label = fst_child->size > snd_child->size ? fst_child->label
                                                        : snd_child->label;

        dendro->label = malloc(strlen(label) * sizeof(char));
        if (dendro->label == NULL) {
            return -1;
        }
        strncpy(dendro->label, label, strlen(label) * sizeof(char));
    }
    return 0;
}

int
cluster_hierarchical(list_ul_t* nodes_of_part,
                     dict_ul_ul_t** dif_sets,
                     dendrogram_t** blocks,
                     unsigned long* block_count)
{
    size_t n_nodes_of_p = list_ul_size(nodes_of_part);
    float* pairwise_diff = calloc(n_nodes_of_p * n_nodes_of_p, sizeof(float));
    size_t n_clusters = n_nodes_of_p;
    unsigned int* h_part = calloc(n_nodes_of_p, sizeof(unsigned int));
    unsigned long min_idx[2];
    dendrogram_t** dendros = malloc(n_nodes_of_p * sizeof(dendrogram_t*));
    dendrogram_t* dendro;
    blocks = malloc(list_ul_size(nodes_of_part) * sizeof(dendrogram_t*));
    size_t length;

    if (pairwise_diff == NULL || h_part == NULL || dendros == NULL ||
        blocks == NULL) {
        return -1;
    }

    // Compute pairwise distance matrix
    for (size_t i = 0; i < n_nodes_of_p; ++i) {
        for (size_t j = 0; j < n_nodes_of_p - i; ++j) {
            if (i == j) {
                continue;
            }
            pairwise_diff[j * n_nodes_of_p + j] =
                  weighted_jaccard_dist(dif_sets[i], dif_sets[j]);
            pairwise_diff[i * n_nodes_of_p + j] =
                  pairwise_diff[j * n_nodes_of_p + i];
        }
    }

    for (size_t i = 0; i < n_nodes_of_p; ++i) {
        h_part[i] = i;
        dendros[i] = malloc(sizeof(dendrogram_t));
        dendros[i]->children.node = i;
        length = snprintf(NULL, 0, "%lu", i);
        dendros[i]->label = malloc(length + 1);
        snprintf(dendros[i]->label, length, "%lu", i);
        dendros[i]->size = sizeof(node_t);
        dendros[i]->uncapt_s = dendros[i]->size;
    }

    while (n_clusters > 1) {
        find_min_dist(pairwise_diff, n_nodes_of_p, min_idx, dendros);

        dendro = malloc(sizeof(dendrogram_t));
        if (dendro == NULL) {
            return -1;
        }
        dendro->children.dendro[0] = dendros[min_idx[0]];
        dendro->children.dendro[1] = dendros[min_idx[1]];
        dendro->size = dendros[min_idx[0]]->size + dendros[min_idx[1]]->size;
        dendro->uncapt_s =
              dendros[min_idx[0]]->uncapt_s + dendros[min_idx[1]]->uncapt_s;

        if (dendro->uncapt_s > BLOCK_SIZE) {
            (*block_count)++;
            dendro->block_no = *block_count;
            blocks[(*block_count) - 1] = dendro;
        }

        assign_dendro_label(dendro, dendros[min_idx[0]], dendros[min_idx[1]]);

        dendros[min_idx[0]] = dendro;
        dendros[min_idx[1]] = dendro;
        n_clusters--;
    }

    free(dendros);
    free(pairwise_diff);
    free(h_part);
    blocks = realloc(blocks, *block_count * sizeof(dendrogram_t*));
    if (blocks == NULL) {
        return -1;
    }

    return 0;
}

int
block_formation(in_memory_file_t* db,
                dict_ul_ul_t** dif_sets,
                const unsigned long* parts,
                dendrogram_t*** blocks,
                unsigned long* block_count)
{
    size_t n_nodes = db->node_id_counter;
    size_t num_clusters = get_num_coarse_clusters(db);
    blocks = malloc(num_clusters * sizeof(dendrogram_t**));
    if (blocks == NULL) {
        return -1;
    }

    list_ul_t* nodes_per_part[num_clusters];

    for (size_t i = 0; i < num_clusters; ++i) {
        nodes_per_part[i] = create_list_ul();
    }

    for (size_t i = 0; i < n_nodes; ++i) {
        list_ul_append(nodes_per_part[parts[i]], i);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        cluster_hierarchical(
              nodes_per_part[i], dif_sets, blocks[i], &(block_count[i]));
    }

    return 0;
}

int
compare_by_label(const void* a, const void* b, void* array2)
{
    dendrogram_t** blocks = (dendrogram_t**)array2;
    dendrogram_t* block_a = blocks[*(unsigned long*)a];
    dendrogram_t* block_b = blocks[*(unsigned long*)b];
    long diff = strcmp(block_a->label, block_b->label);
    if (diff == 0) {
        diff = (long)(block_a->block_no - block_b->block_no);
    }
    return (0 < diff) - (diff < 0);
}

// FIXME does not order the blocks properly, does it?
unsigned long*
sort_by_label(dendrogram_t** blocks, unsigned long size)
{
    unsigned long* order = malloc(size * sizeof(unsigned long));
    if (order == NULL) {
        exit(-1);
    }

    for (size_t i = 0; i < size; i++) {
        order[i] = i;
    }
    qsort_r(
          order, size, sizeof(unsigned long), compare_by_label, (void*)blocks);
    return order;
}

int
layout_blocks(in_memory_file_t* db,
              dendrogram_t*** blocks,
              unsigned long* block_count)
{
    size_t num_clusters = get_num_coarse_clusters(db);
    unsigned long* order[num_clusters];

    for (size_t i = 0; i < num_clusters; ++i) {
        order[i] = sort_by_label(blocks[i], block_count[i]);
    }

    // TODO distance for subgraphs based on edges
    // use function to generate distance in cluster hierarchical
    // apply cluster hierarchical to subgraphs
    // sort like in the lines above
    // impl mapping function for blocks 2 node ids

    // map_blocks_to_node_ids(blocks, order);
    return 0;
}
