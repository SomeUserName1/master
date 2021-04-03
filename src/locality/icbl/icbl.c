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
#include "../../data-struct/cbs.h"
#include "../../data-struct/list.h"
#include "../../data-struct/list_ul.h"
#include "../../query/degree.h"
#include "../../query/random_walk.h"

#define SHARE_OF_MEMORY (0.8)

static size_t
get_num_walks(in_memory_file_t* db)
{
    size_t min_deg        = get_min_degree(db, BOTH);
    size_t max_deg        = get_max_degree(db, BOTH);
    size_t range          = max_deg - min_deg + 1;
    size_t nodes_per_step = db->node_id_counter / range;
    size_t num_walks      = 1;

    list_relationship_t* rels;

    size_t* degree_hist = calloc(range, sizeof(size_t));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);
        degree_hist[list_relationship_size(rels) - min_deg]++;
        list_relationship_destroy(rels);
    }

    for (size_t i = 0; i < range; ++i) {
        if (degree_hist[i] > nodes_per_step) {
            if (i == 0 && min_deg == 0) {
                num_walks = 1;
            } else {
                num_walks = i + min_deg;
            }
            break;
        }
    }
    free(degree_hist);

    return num_walks;
}

static inline size_t
get_num_coarse_clusters(in_memory_file_t* db)
{
    return ceil(((double)((sizeof(node_t) + log((double)db->node_id_counter))
                          * (double)db->node_id_counter))
                / sqrt(SHARE_OF_MEMORY * MEMORY));
}

static inline size_t
get_num_steps(in_memory_file_t* db)
{
    return 1
           + ceil(log2((float)db->node_id_counter)
                  / get_num_coarse_clusters(db));
}

int
identify_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets)
{
    size_t        num_nodes = db->node_id_counter;
    size_t        num_walks = get_num_walks(db);
    size_t        num_steps = get_num_steps(db);
    path*         result;
    unsigned long node_id;
    list_ul_t*    visited_nodes;

    for (size_t i = 0; i < num_nodes; ++i) {
        for (size_t j = 0; j < num_walks; ++j) {
            result        = random_walk(db, i, num_steps, BOTH);
            visited_nodes = path_extract_vertices(result, db);

            for (size_t k = 0; k < list_ul_size(visited_nodes); ++k) {
                node_id = list_ul_get(visited_nodes, k);
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
    list_ul_t*             visited_elems_b = create_list_ul();

    unsigned long* key     = NULL;
    unsigned long* value_a = NULL;
    unsigned long* value_b = NULL;

    unsigned long intersect_sum = 0;
    unsigned long union_sum     = 0;

    while (dict_ul_ul_iterator_next(it, &key, &value_a) > -1) {
        if (dict_ul_ul_contains(dif_set_b, *key)) {
            list_ul_append(visited_elems_b, *key);
            *value_b = dict_ul_ul_get_direct(dif_set_b, *key);

            intersect_sum += *value_a > *value_b ? *value_b : *value_a;
            union_sum += *value_a > *value_b ? *value_a : *value_b;
        } else {
            union_sum += *value_a;
        }
    }
    dict_ul_ul_iterator_destroy(it);
    it = create_dict_ul_ul_iterator(dif_set_b);

    while (dict_ul_ul_iterator_next(it, &key, &value_b) > -1) {
        if (!list_ul_contains(visited_elems_b, *key)) {
            union_sum += *value_b;
        }
    }

    dict_ul_ul_iterator_destroy(it);
    list_ul_destroy(visited_elems_b);

    return 1 - ((float)intersect_sum / (float)union_sum);
}

static void
insert_match(size_t*       max_degree_nodes,
             size_t*       max_degrees,
             unsigned long node_id,
             unsigned long degree,
             size_t        num_clusters)
{
    bool          placed = false;
    unsigned long temp_d;
    unsigned long temp1_d;
    unsigned long temp_id;
    unsigned long temp1_id;

    for (size_t i = 0; i < num_clusters; ++i) {
        if (placed) {
            temp1_id            = max_degree_nodes[i];
            temp1_d             = max_degrees[i];
            max_degree_nodes[i] = temp_id;
            max_degrees[i]      = temp_d;
            temp_id             = temp1_id;
            temp_d              = temp1_d;
        } else if (degree > max_degrees[i]) {
            temp_id             = max_degree_nodes[i];
            temp_d              = max_degrees[i];
            max_degree_nodes[i] = node_id;
            max_degrees[i]      = degree;
            placed              = true;
        }
    }
}

int
initialize_centers(in_memory_file_t* db,
                   unsigned long**   centers,
                   size_t            num_clusters)
{
    if (!db || !centers) {
        printf("icbl.c: initialize_centers: Invalid Argument!\n");
        exit(-1);
    }

    unsigned long  num_nodes = db->node_id_counter;
    unsigned long* max_degree_nodes =
          calloc(num_clusters, sizeof(unsigned long));
    unsigned long* max_degrees = calloc(num_clusters, sizeof(unsigned long));
    list_relationship_t* rels;
    unsigned long        degree;

    if (!max_degrees || !max_degree_nodes) {
        printf("icbl.c: initialize_centers: Memory allocation failed!\n");
        exit(-1);
    }
    // FIXME check distance first
    for (size_t i = 0; i < num_nodes; ++i) {
        rels   = in_memory_expand(db, i, BOTH);
        degree = list_relationship_size(rels);
        if (degree > max_degrees[num_clusters - 1]) {
            insert_match(
                  max_degree_nodes, max_degrees, i, degree, num_clusters);
        }
        list_relationship_destroy(rels);
    }
    free(max_degrees);

    for (size_t i = 0; i < num_clusters; ++i) {
        printf("max_degree_nodes: %lu\n", max_degree_nodes[i]);
    }
    *centers = max_degree_nodes;
    return 0;
}

size_t
assign_to_cluster(size_t               num_nodes,
                  dict_ul_ul_t**       dif_sets,
                  unsigned long*       part,
                  const unsigned long* centers,
                  size_t               num_clusters)
{
    float         min_dist = FLT_MAX;
    float         dist;
    unsigned long best_center_idx = ULONG_MAX;

    for (size_t i = 0; i < num_nodes; ++i) {
        for (size_t j = 0; j < num_clusters; ++j) {
            dist = weighted_jaccard_dist(dif_sets[centers[j]], dif_sets[i]);
            if (dist < min_dist) {
                best_center_idx = j;
            }
        }
        part[i] = best_center_idx;

        min_dist        = 0;
        best_center_idx = ULONG_MAX;
    }

    return 0;
}

int
updated_centers(size_t               num_nodes,
                dict_ul_ul_t**       dif_sets,
                const unsigned long* part,
                unsigned long*       centers,
                size_t               num_clusters)
{
    if (!dif_sets || !part || !centers || num_clusters < 1) {
        printf("icbl.c: update_centers: Invalid Argument!\n");
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
    unsigned long*         node_id;
    unsigned long*         count = 0;

    unsigned long max_node_id = UNINITIALIZED_LONG;
    unsigned int  max_count   = 0;

    for (size_t i = 0; i < num_nodes; ++i) {
        it = create_dict_ul_ul_iterator(dif_sets[i]);

        while (dict_ul_ul_iterator_next(it, &node_id, &count) > -1) {
            if (dict_ul_ul_contains(cluster_counts[part[i]], *node_id)) {

                dict_ul_ul_insert(
                      cluster_counts[part[i]],
                      *node_id,
                      dict_ul_ul_get_direct(cluster_counts[part[i]], *node_id)
                            + *count);

            } else {
                dict_ul_ul_insert(dif_sets[part[i]], *node_id, *count);
            }
        }
        dict_ul_ul_iterator_destroy(it);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        it = create_dict_ul_ul_iterator(cluster_counts[i]);
        while (dict_ul_ul_iterator_next(it, &node_id, &count) > -1) {
            if (*count > max_count) {
                max_node_id = *node_id;
            }
        }
        centers[i]  = max_node_id;
        max_count   = 0;
        max_node_id = UNINITIALIZED_LONG;
        dict_ul_ul_iterator_destroy(it);
        dict_ul_ul_destroy(cluster_counts[i]);
    }

    free(cluster_counts);

    return 0;
}

int
cluster_coarse(in_memory_file_t* db,
               dict_ul_ul_t**    dif_sets,
               unsigned long*    part)
{
    if (!db || !dif_sets || !part) {
        exit(-1);
    }

    size_t num_clusters = get_num_coarse_clusters(db);
    size_t changes      = 0;

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
find_min_dist(const float*   pairwise_diff,
              size_t         n_nodes,
              unsigned long* idx,
              dendrogram_t** dendros)
{
    float min_dist = FLT_MAX;

    // Compute pairwise distance matrix
    for (size_t i = 0; i < n_nodes; ++i) {
        for (size_t j = 0; j < n_nodes - i; ++j) {
            // Checking if both are the same dendrogram ensures, that already
            // merged dendrogram are not merged again
            if (i == j || dendros[i] == dendros[j]) {
                continue;
            }
            if (pairwise_diff[i * n_nodes + j] < min_dist) {
                min_dist = pairwise_diff[i * n_nodes + j];
                idx[0]   = i;
                idx[1]   = j;
            }
        }
    }
    return 0;
}

dendrogram_t**
initialize_node_dendrograms(list_ul_t* nodes_of_p)
{
    unsigned long  n_nodes_of_p = list_ul_size(nodes_of_p);
    dendrogram_t** dendros      = malloc(n_nodes_of_p * sizeof(dendrogram_t*));
    unsigned long  n_id;
    size_t         length;

    for (size_t i = 0; i < n_nodes_of_p; ++i) {
        n_id                      = list_ul_get(nodes_of_p, i);
        dendros[i]                = malloc(sizeof(dendrogram_t));
        dendros[i]->children.node = n_id;
        length                    = snprintf(NULL, 0, "%lu", n_id);
        dendros[i]->label         = malloc(length + 1);
        snprintf(dendros[i]->label, length, "%lu", n_id);
        dendros[i]->size     = sizeof(node_t);
        dendros[i]->uncapt_s = dendros[i]->size;
        dendros[i]->block_no = ULONG_MAX;
    }

    return dendros;
}

float*
node_distance_matrix(dict_ul_ul_t** dif_sets, list_ul_t* nodes_of_p)
{
    unsigned long n_nodes_of_p = list_ul_size(nodes_of_p);
    float* pairwise_dist = calloc(n_nodes_of_p * n_nodes_of_p, sizeof(float));

    // Compute pairwise distance matrix
    for (size_t i = 0; i < n_nodes_of_p; ++i) {
        for (size_t j = 0; j < n_nodes_of_p - i; ++j) {
            if (i == j) {
                continue;
            }
            pairwise_dist[j * n_nodes_of_p + j] =
                  weighted_jaccard_dist(dif_sets[list_ul_get(nodes_of_p, i)],
                                        dif_sets[list_ul_get(nodes_of_p, j)]);

            pairwise_dist[i * n_nodes_of_p + j] =
                  pairwise_dist[j * n_nodes_of_p + i];
        }
    }

    return pairwise_dist;
}

int
assign_dendro_label(dendrogram_t* dendro,
                    dendrogram_t* fst_child,
                    dendrogram_t* snd_child,
                    bool          new_block)
{
    if (fst_child->size > BLOCK_SIZE && snd_child->size > BLOCK_SIZE) {
        // block label of first + : + block label of snd + \0
        size_t length =
              strlen(fst_child->label) + 1 + strlen(snd_child->label) + 1;
        dendro->label = malloc(length * sizeof(char));
        if (dendro->label == NULL) {
            exit(-1);
        }
        dendro->label[0] = '\0';

        if (fst_child->size > snd_child->size) {
            strncat(dendro->label, fst_child->label, strlen(fst_child->label));
            strncat(dendro->label, ":", 1);
            strncat(dendro->label, snd_child->label, strlen(snd_child->label));
        } else {
            strncat(dendro->label, snd_child->label, strlen(snd_child->label));
            strncat(dendro->label, ":", 1);
            strncat(dendro->label, fst_child->label, strlen(fst_child->label));
        }
        dendro->label[length] = '\0';
    } else {
        char* label = fst_child->size > snd_child->size ? fst_child->label
                                                        : snd_child->label;
        if (new_block) {
            size_t block_num_length =
                  snprintf(NULL, 0, "%lu", dendro->block_no);

            dendro->label = malloc((strlen(label) + 1 + block_num_length + 1)
                                   * sizeof(char));

            if (dendro->label == NULL) {
                return -1;
            }

            strncpy(dendro->label, label, strlen(label) * sizeof(char));

            dendro->label[strlen(label)] = '.';

            snprintf(dendro->label + strlen(label) + 1,
                     block_num_length,
                     "%lu",
                     dendro->block_no);
        } else {
            dendro->label = label;
        }
    }
    return 0;
}

int
cluster_hierarchical(unsigned long  n_nodes,
                     float*         dist,
                     dendrogram_t** dendros,
                     bool           block_formation,
                     dendrogram_t** blocks,
                     unsigned long* block_count)
{
    size_t        n_clusters = n_nodes;
    unsigned long min_idx[2];
    dendrogram_t* dendro;

    if (dist == NULL || dendros == NULL) {
        exit(-1);
    }

    if (block_formation) {
        blocks = malloc(n_nodes * sizeof(dendrogram_t*));
        if (blocks == NULL) {
            exit(-1);
        }
    }

    while (n_clusters > 1) {
        find_min_dist(dist, n_nodes, min_idx, dendros);

        dendro = malloc(sizeof(dendrogram_t));
        if (dendro == NULL) {
            exit(-1);
        }
        dendro->children.dendro[0] = dendros[min_idx[0]];
        dendro->children.dendro[1] = dendros[min_idx[1]];
        dendro->size = dendros[min_idx[0]]->size + dendros[min_idx[1]]->size;
        dendro->uncapt_s =
              dendros[min_idx[0]]->uncapt_s + dendros[min_idx[1]]->uncapt_s;

        if (block_formation) {
            if (dendro->uncapt_s > BLOCK_SIZE) {
                (*block_count)++;
                dendro->block_no           = *block_count;
                blocks[(*block_count) - 1] = dendro;
                assign_dendro_label(
                      dendro, dendros[min_idx[0]], dendros[min_idx[1]], true);
                dendro->uncapt_s = 0;
            } else {
                assign_dendro_label(
                      dendro, dendros[min_idx[0]], dendros[min_idx[1]], false);
            }
        }

        dendros[min_idx[0]] = dendro;
        dendros[min_idx[1]] = dendro;
        n_clusters--;
    }

    free(dendros);
    free(dist);

    if (block_formation) {
        blocks = realloc(blocks, *block_count * sizeof(dendrogram_t*));
        if (blocks == NULL) {
            exit(-1);
        }
    } else {
        blocks = dendros;
    }

    return 0;
}

int
block_formation(in_memory_file_t*    db,
                dict_ul_ul_t**       dif_sets,
                const unsigned long* parts,
                dendrogram_t***      blocks,
                unsigned long*       block_count)
{
    size_t n_nodes      = db->node_id_counter;
    size_t num_clusters = get_num_coarse_clusters(db);
    blocks              = malloc(num_clusters * sizeof(dendrogram_t**));
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
        cluster_hierarchical(list_ul_size(nodes_per_part[i]),
                             node_distance_matrix(dif_sets, nodes_per_part[i]),
                             initialize_node_dendrograms(nodes_per_part[i]),
                             true,
                             blocks[i],
                             &(block_count[i]));
    }

    return 0;
}

int
compare_by_label(const void* a, const void* b, void* array2)
{
    dendrogram_t** blocks  = (dendrogram_t**)array2;
    dendrogram_t*  block_a = blocks[*(unsigned long*)a];
    dendrogram_t*  block_b = blocks[*(unsigned long*)b];
    long           diff    = strcmp(block_a->label, block_b->label);
    if (diff == 0) {
        diff = (long)(block_a->block_no - block_b->block_no);
    }
    return (0 < diff) - (diff < 0);
}

void
sort_by_label(dendrogram_t** blocks, unsigned long size)
{
    unsigned long* order = malloc(size * sizeof(unsigned long));
    if (order == NULL) {
        exit(-1);
    }

    for (size_t i = 0; i < size; i++) {
        order[i] = i;
    }
    // Sort by labels
    qsort_r(
          order, size, sizeof(unsigned long), compare_by_label, (void*)blocks);

    // Reorder blocks
    dendrogram_t* temp_dendro;
    for (size_t j = 0; j < size; ++j) {
        temp_dendro      = blocks[j];
        blocks[j]        = blocks[order[j]];
        blocks[order[j]] = temp_dendro;
    }

    free(order);
}

float*
subgraph_distance(in_memory_file_t*    db,
                  const unsigned long* partition,
                  size_t               num_clusters)
{
    list_relationship_t* rels = in_memory_get_relationships(db);
    relationship_t*      rel;
    float* subgraph_dist = calloc(num_clusters * num_clusters, sizeof(long));

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        if (partition[rel->source_node] != partition[rel->target_node]) {
            subgraph_dist[partition[rel->source_node]
                          + num_clusters * partition[rel->target_node]]++;
            subgraph_dist[partition[rel->target_node]
                          + num_clusters * partition[rel->source_node]]++;
        }
    }
    return subgraph_dist;
}

dendrogram_t**
initialize_subgraph_dendrograms(unsigned long n_subgraphs)
{
    dendrogram_t** dendros = malloc(n_subgraphs * sizeof(dendrogram_t*));
    size_t         length;

    for (size_t i = 0; i < n_subgraphs; ++i) {
        dendros[i]                = malloc(sizeof(dendrogram_t));
        dendros[i]->children.node = i;
        length                    = snprintf(NULL, 0, "%lu", i);
        dendros[i]->label         = malloc(length + 1);
        snprintf(dendros[i]->label, length, "%lu", i);
        dendros[i]->size     = 1;
        dendros[i]->uncapt_s = dendros[i]->size;
    }

    return dendros;
}

void
order_subgraphs(dendrogram_t*   hierarchy,
                dendrogram_t*** blocks,
                unsigned long   n_subgraphs)
{
    unsigned long* subgraph_order =
          malloc(n_subgraphs * sizeof(*subgraph_order));
    unsigned long rank = 0;

    list_cbs_t    cbs   = { ptr_eq, NULL, NULL };
    list_t*       stack = create_list(&cbs);
    dendrogram_t* current;

    list_append(stack, (void*)&hierarchy);

    while (list_size(stack) > 0) {
        current = (dendrogram_t*)list_take(stack, list_size(stack) - 1);

        if (current->size == 1) {
            subgraph_order[rank] = current->children.node;
            rank++;
        }

        list_append(stack, (void*)&(current->children.dendro[0]));
        list_append(stack, (void*)&(current->children.dendro[1]));
    }

    dendrogram_t** temp_dendro;
    for (size_t i = 0; i < n_subgraphs; ++i) {
        temp_dendro               = blocks[i];
        blocks[i]                 = blocks[subgraph_order[i]];
        blocks[subgraph_order[i]] = temp_dendro;
    }
    free(subgraph_order);
}

void
map_to_partitions(unsigned long*       partition,
                  dendrogram_t***      blocks,
                  const unsigned long* block_count,
                  unsigned long        num_subgraphs)
{
    unsigned long partition_counter = 0;
    list_cbs_t    cbs               = { ptr_eq, NULL, NULL };
    list_t*       stack             = create_list(&cbs);
    dendrogram_t* current;

    for (size_t i = 0; i < num_subgraphs; ++i) {
        for (size_t j = 0; j < block_count[i]; ++j) {
            list_append(stack, blocks[i][j]);

            while (list_size(stack) > 0) {
                current = (dendrogram_t*)list_take(stack, list_size(stack) - 1);

                // if one of the children of the current dendrogram node is a
                // block on its own, dont assign a partition number and dont put
                // its children on the stack; they will be handled in a later or
                // have been handled in a previous iteration.
                if (current->block_no != ULONG_MAX) {
                    continue;
                }

                if (current->size == 1) {
                    partition[current->children.node] = partition_counter;
                }

                list_append(stack, (void*)&(current->children.dendro[0]));
                list_append(stack, (void*)&(current->children.dendro[1]));
            }
            partition_counter++;
        }
    }
}

int
layout_blocks(in_memory_file_t* db,
              dendrogram_t***   blocks,
              unsigned long*    block_count,
              unsigned long*    partition)
{
    size_t num_clusters = get_num_coarse_clusters(db);

    for (size_t i = 0; i < num_clusters; ++i) {
        sort_by_label(blocks[i], block_count[i]);
    }

    dendrogram_t** hierarchy = NULL;
    cluster_hierarchical(num_clusters,
                         subgraph_distance(db, partition, num_clusters),
                         initialize_subgraph_dendrograms(num_clusters),
                         false,
                         hierarchy,
                         NULL);
    order_subgraphs(hierarchy[0], blocks, num_clusters);

    map_to_partitions(partition, blocks, block_count, num_clusters);

    return 0;
}

unsigned long*
icbl(in_memory_file_t* db)
{
    dict_ul_ul_t** diff_sets =
          calloc(db->node_id_counter, sizeof(dict_ul_ul_t*));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        diff_sets[i] = create_dict_ul_ul();
    }

    identify_diffustion_sets(db, diff_sets);

    unsigned long* partition =
          malloc(db->node_id_counter * sizeof(unsigned long));
    cluster_coarse(db, diff_sets, partition);

    dendrogram_t*** blocks      = NULL;
    unsigned long*  block_count = NULL;
    block_formation(db, diff_sets, partition, blocks, block_count);

    layout_blocks(db, blocks, block_count, partition);

    return partition;
}
