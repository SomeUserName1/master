#include "icbl.h"

#define _GNU_SOURCE

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../constants.h"
#include "../../data-struct/cbs.h"
#include "../../data-struct/list.h"
#include "../../data-struct/list_ul.h"
#include "../../data-struct/set_ul.h"
#include "../../query/degree.h"
#include "../../query/random_walk.h"

inline size_t
get_num_walks(in_memory_file_t* db)
{
    if (!db) {
        printf("ICBL - get_num_walks: Invalid Arguments!\n");
        exit(-1);
    }

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
            num_walks = i + min_deg;
            break;
        }
    }
    free(degree_hist);

    return num_walks > 0 ? num_walks : 1;
}

static inline size_t
get_num_coarse_clusters(in_memory_file_t* db)
{
    if (!db) {
        printf("ICBL - get_num_coarse_clusters: Invalid Arguments!\n");
        exit(-1);
    }

    size_t result = ceil(((sizeof(node_t) * (double)db->node_id_counter)
                          + sizeof(relationship_t) * (double)db->rel_id_counter)
                         / sqrt(SHARE_OF_MEMORY * MEMORY));
    return result + 1;
}

inline size_t
get_num_steps(in_memory_file_t* db)
{
    if (!db) {
        printf("ICBL - get_num_steps: Invalid Arguments!\n");
        exit(-1);
    }

    return (unsigned long)1
           + ceil(log2((float)db->node_id_counter)
                  / get_num_coarse_clusters(db));
}

void
dendrogram_destroy(dendrogram_t* root)
{
    list_cbs_t    cbs   = { ptr_eq, NULL, NULL };
    list_t*       stack = create_list(&cbs);
    dendrogram_t* current;

    list_append(stack, root);

    while (list_size(stack) > 0) {
        current = (dendrogram_t*)list_take(stack, list_size(stack) - 1);

        if (current->id == UNINITIALIZED_LONG) {
            list_append(stack, current->children.dendro[0]);
            list_append(stack, current->children.dendro[1]);
        }
        free(current->label);
        free(current);
    }
    list_destroy(stack);
}

void
dendrogram_print(dendrogram_t* root)
{
    list_cbs_t    cbs   = { ptr_eq, NULL, NULL };
    list_t*       stack = create_list(&cbs);
    dendrogram_t* current;

    list_append(stack, root);

    while (list_size(stack) > 0) {
        current = (dendrogram_t*)list_take(stack, list_size(stack) - 1);

        printf("current: %p, \t size: %lu, \t block? %lu",
               current,
               current->size,
               current->block_no == ULONG_MAX ? -1 : current->block_no);
        if (current->id == UNINITIALIZED_LONG) {
            printf("\t child1: %p, \t child2: %p",
                   current->children.dendro[0],
                   current->children.dendro[1]);
            list_append(stack, current->children.dendro[0]);
            list_append(stack, current->children.dendro[1]);
        } else {
            printf("\t node %lu", current->children.node);
        }
        printf("\n");
    }
    list_destroy(stack);
}

void
identify_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets)
{
    if (!db || !dif_sets) {
        printf("ICBL - identify_diffustion_sets: Invalid Arguments!\n");
        exit(-1);
    }

    size_t        num_nodes = db->node_id_counter;
    size_t        num_walks = get_num_walks(db);
    size_t        num_steps = get_num_steps(db);
    path*         result;
    unsigned long node_id;
    list_ul_t*    visited_nodes;

    srand(time(NULL));

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
            path_destroy(result);
            list_ul_destroy(visited_nodes);
        }
    }
}

float
weighted_jaccard_dist(dict_ul_ul_t* dif_set_a, dict_ul_ul_t* dif_set_b)
{
    if (!dif_set_a || !dif_set_b) {
        printf("ICBL - weighted_jaccard_dist: Invalid Arguments!\n");
        exit(-1);
    }

    dict_ul_ul_iterator_t* it          = create_dict_ul_ul_iterator(dif_set_a);
    set_ul_t*              seen_keys_b = create_set_ul();

    unsigned long* key     = NULL;
    unsigned long* value_a = NULL;
    unsigned long  value_b;
    unsigned long* value_b_ptr = &value_b;

    unsigned long intersect_sum = 0;
    unsigned long union_sum     = 0;

    while (dict_ul_ul_iterator_next(it, &key, &value_a) > -1) {
        if (dict_ul_ul_contains(dif_set_b, *key)) {
            set_ul_insert(seen_keys_b, *key);
            value_b = dict_ul_ul_get_direct(dif_set_b, *key);

            intersect_sum += *value_a > value_b ? value_b : *value_a;
            union_sum += *value_a > value_b ? *value_a : value_b;
        } else {
            union_sum += *value_a;
        }
    }
    dict_ul_ul_iterator_destroy(it);
    it = create_dict_ul_ul_iterator(dif_set_b);

    while (dict_ul_ul_iterator_next(it, &key, &value_b_ptr) > -1) {
        if (!set_ul_contains(seen_keys_b, *key)) {
            union_sum += *value_b_ptr;
        }
    }

    dict_ul_ul_iterator_destroy(it);
    set_ul_destroy(seen_keys_b);

    return 1 - ((float)intersect_sum / (float)union_sum);
}

void
insert_match(size_t*       max_degree_nodes,
             size_t*       max_degrees,
             unsigned long node_id,
             unsigned long degree,
             size_t        num_clusters)
{
    if (!max_degree_nodes || !max_degrees) {
        printf("ICBL - insert_match: Invalid Arguments!\n");
        exit(-1);
    }

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

bool
check_dist_bound(const size_t*  max_degree_nodes,
                 size_t         candidate,
                 unsigned long  num_found,
                 dict_ul_ul_t** dif_sets)
{
    if (!max_degree_nodes || !dif_sets) {
        printf("ICBL - check dist bound: Invalid Arguments!\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_found; ++i) {
        if (weighted_jaccard_dist(dif_sets[max_degree_nodes[i]],
                                  dif_sets[candidate])
            < MIN_DIST_INIT_CENTERS) {
            return false;
        }
    }
    return true;
}

void
initialize_centers(in_memory_file_t* db,
                   unsigned long**   centers,
                   size_t            num_clusters,
                   dict_ul_ul_t**    dif_sets)
{
    if (!db || !centers) {
        printf("ICBL - initialize_centers: Invalid Argument!\n");
        exit(-1);
    }

    unsigned long        num_nodes = db->node_id_counter;
    list_relationship_t* rels;
    unsigned long        degree;
    unsigned long        num_found = 0;

    unsigned long* max_degrees = calloc(num_clusters, sizeof(unsigned long));
    unsigned long* max_degree_nodes =
          calloc(num_clusters, sizeof(unsigned long));

    if (!max_degrees || !max_degree_nodes) {
        printf("icbl.c: initialize_centers: Memory allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_nodes; ++i) {
        rels   = in_memory_expand(db, i, BOTH);
        degree = list_relationship_size(rels);
        if (degree > max_degrees[num_clusters - 1]) {
            if (!check_dist_bound(max_degree_nodes, i, num_found, dif_sets)) {
                continue;
            }
            insert_match(
                  max_degree_nodes, max_degrees, i, degree, num_clusters);
            if (num_found < num_clusters) {
                num_found++;
            }
        }
        list_relationship_destroy(rels);
    }
    free(max_degrees);

    *centers = max_degree_nodes;
}

size_t
assign_to_cluster(size_t               num_nodes,
                  dict_ul_ul_t**       dif_sets,
                  unsigned long*       part,
                  const unsigned long* centers,
                  size_t               num_clusters)
{
    if (!dif_sets || !part || !centers) {
        printf("ICBL - assign_to_cluster: Invalid Argument!\n");
        exit(-1);
    }

    float         min_dist = FLT_MAX;
    float         dist;
    unsigned long best_center_idx = ULONG_MAX;
    unsigned long changes         = 0;

    if (num_clusters == 1) {
        for (size_t i = 0; i < num_nodes; ++i) {
            part[i] = 0;
        }
    }

    for (size_t i = 0; i < num_nodes; ++i) {
        for (size_t j = 0; j < num_clusters; ++j) {
            dist = weighted_jaccard_dist(dif_sets[centers[j]], dif_sets[i]);
            if (dist < min_dist) {
                min_dist        = dist;
                best_center_idx = j;
            }
        }
        if (best_center_idx < ULONG_MAX) {
            if (part[i] != best_center_idx) {
                changes++;
                part[i] = best_center_idx;
            }
        } else {
            printf("Couldn't find an appropriate center; sth is wrong!\n");
            exit(-1);
        }

        min_dist        = FLT_MAX;
        best_center_idx = ULONG_MAX;
    }

    return changes;
}

void
update_centers(size_t               num_nodes,
               dict_ul_ul_t**       dif_sets,
               const unsigned long* part,
               unsigned long*       centers,
               size_t               num_clusters)
{
    if (!dif_sets || !part || !centers || num_clusters < 1) {
        printf("ICBL - update_centers: Invalid Argument!\n");
        exit(-1);
    }

    dict_ul_ul_t** cluster_counts = calloc(num_clusters, sizeof(dict_ul_ul_t*));

    if (!cluster_counts) {
        printf("ICBL - update centers: Memory Allocation failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        cluster_counts[i] = create_dict_ul_ul();
    }

    dict_ul_ul_iterator_t* it;
    unsigned long*         node_id;
    unsigned long          i_count = 0;
    unsigned long*         count   = &i_count;

    unsigned long max_node_id = UNINITIALIZED_LONG;
    unsigned int  max_count   = 0;

    for (size_t i = 0; i < num_nodes; ++i) {
        it = create_dict_ul_ul_iterator(dif_sets[i]);

        while (dict_ul_ul_iterator_next(it, &node_id, &count) == 0) {
            if (dict_ul_ul_contains(cluster_counts[part[i]], *node_id)) {

                dict_ul_ul_insert(
                      cluster_counts[part[i]],
                      *node_id,
                      dict_ul_ul_get_direct(cluster_counts[part[i]], *node_id)
                            + *count);

            } else {
                dict_ul_ul_insert(cluster_counts[part[i]], *node_id, *count);
            }
        }
        dict_ul_ul_iterator_destroy(it);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        it = create_dict_ul_ul_iterator(cluster_counts[i]);

        while (dict_ul_ul_iterator_next(it, &node_id, &count) == 0) {
            if (*count > max_count) {
                max_node_id = *node_id;
                max_count   = *count;
            }
        }

        if (max_node_id == UNINITIALIZED_LONG) {
            printf("Found no new center! Sth is wrong.\n");
            exit(-1);
        }

        centers[i]  = max_node_id;
        max_count   = 0;
        max_node_id = UNINITIALIZED_LONG;

        dict_ul_ul_iterator_destroy(it);
        dict_ul_ul_destroy(cluster_counts[i]);
    }

    free(cluster_counts);
}

int
cluster_coarse(in_memory_file_t* db,
               dict_ul_ul_t**    dif_sets,
               unsigned long*    part)
{
    if (!db || !dif_sets || !part) {
        printf("ICBL - cluster_coarse: Invalid arguments!\n");
        exit(-1);
    }

    size_t num_clusters = get_num_coarse_clusters(db);
    size_t changes      = 0;

    unsigned long* centers = NULL;

    initialize_centers(db, &centers, num_clusters, dif_sets);

    assign_to_cluster(
          db->node_id_counter, dif_sets, part, centers, num_clusters);

    do {
        update_centers(
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
    if (!pairwise_diff || !idx || !dendros) {
        printf("ICBL - find_min_dist: Invalid Arguments!\n");
        exit(-1);
    }

    float min_dist = FLT_MAX;

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
    if (!nodes_of_p) {
        printf("ICBL - initialize_node_dendrograms: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long  n_nodes_of_p = list_ul_size(nodes_of_p);
    dendrogram_t** dendros      = calloc(n_nodes_of_p, sizeof(dendrogram_t*));
    unsigned long  n_id;
    size_t         length;

    for (size_t i = 0; i < n_nodes_of_p; ++i) {
        n_id                      = list_ul_get(nodes_of_p, i);
        dendros[i]                = malloc(sizeof(dendrogram_t));
        dendros[i]->id            = i;
        dendros[i]->children.node = n_id;
        length                    = snprintf(NULL, 0, "%lu", n_id) + 1;
        dendros[i]->label         = malloc(length);
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
    if (!dif_sets || !nodes_of_p) {
        printf("ICBL - node_distance_matrix: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long n_nodes_of_p = list_ul_size(nodes_of_p);
    float* pairwise_dist = calloc(n_nodes_of_p * n_nodes_of_p, sizeof(float));

    // Compute pairwise distance matrix
    for (size_t i = 0; i < n_nodes_of_p; ++i) {
        for (size_t j = 0; j < i; ++j) {
            if (i == j) {
                continue;
            }
            pairwise_dist[j * n_nodes_of_p + i] =
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
    if (!dendro || !fst_child || !snd_child) {
        printf("ICBL - assign_dendro_label: Invalid Arguments!\n");
        exit(-1);
    }

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
        dendro->label[length - 1] = '\0';
    } else {
        char* label = fst_child->size > snd_child->size ? fst_child->label
                                                        : snd_child->label;
        if (new_block) {
            size_t block_num_length =
                  snprintf(NULL, 0, "%lu", dendro->block_no);

            // node_id + . + block num + \0
            dendro->label = malloc((strlen(label) + 1 + block_num_length + 1)
                                   * sizeof(char));

            if (dendro->label == NULL) {
                return -1;
            }

            strncpy(dendro->label, label, strlen(label) * sizeof(char));

            dendro->label[strlen(label)] = '.';

            snprintf(dendro->label + strlen(label) + 1,
                     block_num_length + 1,
                     "%lu",
                     dendro->block_no);
        } else {
            dendro->label = malloc((strlen(label) + 1) * sizeof(char));

            if (dendro->label == NULL) {
                return -1;
            }

            strncpy(dendro->label, label, (strlen(label) + 1) * sizeof(char));
        }
    }
    return 0;
}

void
mark_subtrees(dendrogram_t* new,
              dendrogram_t** dendros,
              unsigned long  first,
              unsigned long  second)
{
    list_cbs_t    cbs   = { ptr_eq, NULL, NULL };
    list_t*       stack = create_list(&cbs);
    dendrogram_t* current;

    list_append(stack, dendros[first]);
    list_append(stack, dendros[second]);

    while (list_size(stack) > 0) {
        current = (dendrogram_t*)list_take(stack, list_size(stack) - 1);

        if (current->id != UNINITIALIZED_LONG) {
            dendros[current->id] = new;
        } else {
            list_append(stack, current->children.dendro[0]);
            list_append(stack, current->children.dendro[1]);
        }
    }
    list_destroy(stack);
}

int
cluster_hierarchical(unsigned long   n_nodes,
                     float*          dist,
                     dendrogram_t**  dendros,
                     bool            block_formation,
                     dendrogram_t*** blocks,
                     unsigned long*  block_count)
{
    if (!dist || !dendros || !blocks || (block_formation && !block_count)) {
        printf("ICBL - cluster_hierarchical: Invalid Arguments!\n");
        printf("%p, \t %p, \t %p, \t %p\n", dist, dendros, blocks, block_count);
        exit(-1);
    }
    size_t        n_clusters = n_nodes;
    unsigned long min_idx[2];
    dendrogram_t* dendro;

    if (block_formation) {
        *blocks = calloc(n_nodes, sizeof(dendrogram_t*));
        if (!*blocks) {
            printf("ICBL - cluster_hierarchical: Memory allocation failed!\n");
            exit(-1);
        }
    }

    while (n_clusters > 1) {
        find_min_dist(dist, n_nodes, min_idx, dendros);

        dendro = calloc(1, sizeof(dendrogram_t));

        if (dendro == NULL) {
            printf("ICBL - cluster_hierarchical: Memory allocation failed!\n");
            exit(-1);
        }
        dendro->id                 = UNINITIALIZED_LONG;
        dendro->children.dendro[0] = dendros[min_idx[0]];
        dendro->children.dendro[1] = dendros[min_idx[1]];
        dendro->size = dendros[min_idx[0]]->size + dendros[min_idx[1]]->size;
        dendro->uncapt_s =
              dendros[min_idx[0]]->uncapt_s + dendros[min_idx[1]]->uncapt_s;

        if (block_formation) {
            if (dendro->uncapt_s > BLOCK_SIZE) {
                dendro->block_no        = *block_count;
                (*blocks)[*block_count] = dendro;
                (*block_count)++;
                assign_dendro_label(
                      dendro, dendros[min_idx[0]], dendros[min_idx[1]], true);
                dendro->uncapt_s = 0;
            } else {
                dendro->block_no = ULONG_MAX;
                assign_dendro_label(
                      dendro, dendros[min_idx[0]], dendros[min_idx[1]], false);
            }
        }

        mark_subtrees(dendro, dendros, min_idx[0], min_idx[1]);

        dendros[min_idx[0]] = dendro;
        dendros[min_idx[1]] = dendro;
        n_clusters--;
    }

    free(dist);

    if (block_formation) {
        if (*block_count == 0) {
            (*blocks)[0] = dendros[0];
            *block_count = 1;
        }

        *blocks = realloc(*blocks, *block_count * sizeof(dendrogram_t*));

        if (!blocks) {
            free(*blocks);
            printf("ICBL - cluster_hierarchical: Memory Allocation failed!\n");
            exit(-1);
        }
    } else {
        free(*blocks);
        *blocks = dendros;
    }

    return 0;
}

int
block_formation(in_memory_file_t*    db,
                dict_ul_ul_t**       dif_sets,
                const unsigned long* parts,
                dendrogram_t***      blocks,
                unsigned long*       block_count,
                dendrogram_t****     block_roots)
{
    if (!db || !dif_sets || !parts || !blocks || !block_count) {
        printf("ICBL - block_formation: Invalid Arguments!\n");
        exit(-1);
    }

    size_t n_nodes      = db->node_id_counter;
    size_t num_clusters = get_num_coarse_clusters(db);

    list_ul_t* nodes_per_part[num_clusters];

    for (size_t i = 0; i < num_clusters; ++i) {
        nodes_per_part[i] = create_list_ul();
    }

    for (size_t i = 0; i < n_nodes; ++i) {
        list_ul_append(nodes_per_part[parts[i]], i);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        (*block_roots)[i] = initialize_node_dendrograms(nodes_per_part[i]);

        cluster_hierarchical(list_ul_size(nodes_per_part[i]),
                             node_distance_matrix(dif_sets, nodes_per_part[i]),
                             (*block_roots)[i],
                             true,
                             &(blocks[i]),
                             &(block_count[i]));
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        list_ul_destroy(nodes_per_part[i]);
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
    if (!blocks) {
        printf("ICBL - sort_by_label: Invalid Arguments!\n");
        exit(-1);
    }

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
    if (!db || !partition || num_clusters < 1) {
        printf("ICBL - subgraph_distance: Invalid Arguments");
        exit(-1);
    }

    list_relationship_t* rels = in_memory_get_relationships(db);
    relationship_t*      rel;
    float*               subgraph_dist =
          calloc(num_clusters * num_clusters, sizeof(*subgraph_dist));

    for (size_t i = 0; i < list_relationship_size(rels); ++i) {
        rel = list_relationship_get(rels, i);
        if (partition[rel->source_node] != partition[rel->target_node]) {
            subgraph_dist[partition[rel->source_node]
                          + num_clusters * partition[rel->target_node]]++;
            subgraph_dist[partition[rel->target_node]
                          + num_clusters * partition[rel->source_node]]++;
        }
    }
    list_relationship_destroy(rels);

    return subgraph_dist;
}

dendrogram_t**
initialize_subgraph_dendrograms(unsigned long n_subgraphs)
{
    if (n_subgraphs < 1) {
        printf("ICBL - initialize_subgraph_dendrograms: Invalid Arguments!\n");
        exit(-1);
        ;
    }

    dendrogram_t** dendros = malloc(n_subgraphs * sizeof(dendrogram_t*));
    size_t         length;

    if (!dendros) {
        printf("ICBL - initialize_subgraph_dendrograms: Allocating memory "
               "failed\n");
        exit(-1);
    }

    for (size_t i = 0; i < n_subgraphs; ++i) {
        dendros[i] = malloc(sizeof(dendrogram_t));

        if (!dendros[i]) {
            printf("ICBL - initialize_subgraph_dendrograms: Allocating memory "
                   "failed\n");
            exit(-1);
        }
        dendros[i]->id            = i;
        dendros[i]->children.node = i;
        length                    = snprintf(NULL, 0, "%lu", i);
        dendros[i]->label         = malloc(length + 1);

        if (!dendros[i]->label) {
            printf("ICBL - initialize_subgraph_dendrograms: Allocating memory "
                   "failed\n");
            exit(-1);
        }

        snprintf(dendros[i]->label, length, "%lu", i);
        dendros[i]->size     = 1;
        dendros[i]->uncapt_s = dendros[i]->size;
    }

    return dendros;
}

void
order_subgraphs(dendrogram_t**  hierarchy,
                dendrogram_t*** blocks,
                unsigned long   n_subgraphs)
{
    unsigned long* subgraph_order =
          calloc(n_subgraphs, sizeof(*subgraph_order));

    if (!subgraph_order || !hierarchy || !blocks || !*blocks || !**blocks) {
        printf("ICBL - order_subgraphs: Invalid argument or memory allocation "
               "failed!\n");
        exit(-1);
    }

    unsigned long rank = 0;

    list_cbs_t    cbs   = { ptr_eq, NULL, NULL };
    list_t*       stack = create_list(&cbs);
    dendrogram_t* current;

    list_append(stack, *hierarchy);

    while (list_size(stack) > 0) {
        current = (dendrogram_t*)list_take(stack, list_size(stack) - 1);

        if (current->size == 1) {
            subgraph_order[rank] = current->children.node;
            rank++;
        } else {
            list_append(stack, current->children.dendro[0]);
            list_append(stack, current->children.dendro[1]);
        }
    }
    list_destroy(stack);

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
    if (!partition || !blocks || !*blocks || !**blocks) {
        printf("ICBL - map_to_partitions: Invalid arguments failed!\n");
        exit(-1);
    }
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

                if (current->id != UNINITIALIZED_LONG) {
                    partition[current->children.node] = partition_counter;
                } else {
                    if (current->children.dendro[0]->block_no == ULONG_MAX) {
                        list_append(stack, current->children.dendro[0]);
                    }
                    if (current->children.dendro[1]->block_no == ULONG_MAX) {
                        list_append(stack, current->children.dendro[1]);
                    }
                }
            }
            partition_counter++;
        }
    }
    list_destroy(stack);
}

int
layout_blocks(in_memory_file_t* db,
              dendrogram_t***   blocks,
              unsigned long*    block_count,
              unsigned long*    partition,
              dendrogram_t***   block_roots)
{
    if (!db || !blocks || !*blocks || !**blocks || !block_count || !partition) {
        printf("ICBL - layout_blocks: Invalid Arguments!\n");
        exit(-1);
    }
    size_t num_clusters = get_num_coarse_clusters(db);

    for (size_t i = 0; i < num_clusters; ++i) {
        sort_by_label(blocks[i], block_count[i]);
    }

    dendrogram_t** hierarchy = NULL;
    dendrogram_t** dendros   = initialize_subgraph_dendrograms(num_clusters);
    cluster_hierarchical(num_clusters,
                         subgraph_distance(db, partition, num_clusters),
                         dendros,
                         false,
                         &hierarchy,
                         NULL);
    order_subgraphs(hierarchy, blocks, num_clusters);

    map_to_partitions(partition, blocks, block_count, num_clusters);

    for (size_t i = 0; i < num_clusters; ++i) {
        dendrogram_destroy(block_roots[i][0]);
        free(block_roots[i]);
        free(blocks[i]);
    }
    free(block_roots);

    dendrogram_destroy(dendros[0]);
    free(dendros);

    return 0;
}

unsigned long*
icbl(in_memory_file_t* db)
{
    if (!db) {
        printf("ICBL - icbl: Invalid Arguments!\n");
        exit(-1);
    }
    dict_ul_ul_t** diff_sets =
          calloc(db->node_id_counter, sizeof(dict_ul_ul_t*));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        diff_sets[i] = create_dict_ul_ul();
    }

    identify_diffustion_sets(db, diff_sets);

    unsigned long* partition =
          calloc(db->node_id_counter, sizeof(unsigned long));

    cluster_coarse(db, diff_sets, partition);

    unsigned long   num_clusters = get_num_coarse_clusters(db);
    dendrogram_t*** blocks       = calloc(num_clusters, sizeof(dendrogram_t**));
    unsigned long*  block_count  = calloc(num_clusters, sizeof(*block_count));
    dendrogram_t*** block_roots  = calloc(num_clusters, sizeof(dendrogram_t**));

    block_formation(
          db, diff_sets, partition, blocks, block_count, &block_roots);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dict_ul_ul_destroy(diff_sets[i]);
    }
    free(diff_sets);

    layout_blocks(db, blocks, block_count, partition, block_roots);

    free(blocks);
    free(block_count);

    return partition;
}
