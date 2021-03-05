#include "icbl.h"

#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include "../../query/degree.h"
#include "../../data-struct/list_ul.h"
#include "../../query/random_walk.h"
#include "../../constants.h"

#define SHARE_OF_MEMORY (0.8)

static size_t get_num_walks(in_memory_file_t* db) {
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

static inline size_t get_num_coarse_clusters(in_memory_file_t* db) {
    return ceil(((double) (sizeof(node_t) * db->node_id_counter))
            / sqrt(SHARE_OF_MEMORY * MEMORY));
}

static inline size_t get_num_steps(in_memory_file_t* db) {
    return 1 + ceil(log2((float) db->node_id_counter)
            / get_num_coarse_clusters(db));
}

int gen_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets) {
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
                    dict_ul_ul_insert(dif_sets[i], node_id,
                            dict_ul_ul_get_direct(dif_sets[i], node_id) + 1);
                } else {
                    dict_ul_ul_insert(dif_sets[i], node_id, 1);
                }
            }
        }
    }
    return 0;
}

static float weighted_jaccard_dist(dict_ul_ul_t* dif_set_a,
        dict_ul_ul_t* dif_set_b) {
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

    return 1 - ((float) intersect_sum / (float) union_sum);
}

static void insert_match(size_t* max_degree_nodes, size_t* max_degrees,
        unsigned long node_id, unsigned long degree, size_t num_clusters) {
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

int initialize_centers(in_memory_file_t* db, unsigned long** centers,
        size_t num_clusters) {
    unsigned long num_nodes = db->node_id_counter;
    unsigned long* max_degree_nodes = calloc(num_clusters,
            sizeof(unsigned long));
    unsigned long* max_degrees = calloc(num_clusters,
            sizeof(unsigned long));
    list_relationship_t* rels;
    unsigned long degree;

    for (size_t i = 0; i < num_nodes; ++i) {
        rels = in_memory_expand(db, i, BOTH);
        degree = list_relationship_size(rels);
        if (degree > max_degrees[num_clusters - 1]) {
            insert_match(max_degree_nodes, max_degrees, i, num_clusters,
                    degree);
        }
    }
    list_relationship_destroy(rels);

    centers = &max_degree_nodes;
    return 0;
}

size_t assign_to_cluster(size_t num_nodes, dict_ul_ul_t** dif_sets,
        unsigned long* part, const unsigned long* centers, size_t num_clusters) {
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

int updated_centers(size_t num_nodes, dict_ul_ul_t** dif_sets,
        const unsigned long* part, unsigned long* centers, size_t num_clusters) {

    dict_ul_ul_t** cluster_counts = malloc(num_clusters * sizeof(dict_ul_ul_t*));
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
                dict_ul_ul_insert(cluster_counts[part[i]], node_id,
                        dict_ul_ul_get_direct(cluster_counts[part[i]], node_id)
                        + count);
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

    return 0;
}

int cluster_coarse(in_memory_file_t* db, dict_ul_ul_t** dif_sets,
        unsigned long* part) {
    size_t num_clusters = get_num_coarse_clusters(db);
    size_t changes = 0;

    unsigned long* centers = NULL;

    initialize_centers(db, &centers, num_clusters);
    changes = assign_to_cluster(db->node_id_counter, dif_sets, part, centers,
            num_clusters);

    do {
        updated_centers(db->node_id_counter, dif_sets, part, centers, num_clusters);
        changes = assign_to_cluster(db->node_id_counter, dif_sets, part, centers, num_clusters);
    } while (changes > 0);

    return 0;
}
