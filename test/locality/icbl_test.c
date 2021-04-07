#include "../../src/locality/icbl/icbl.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/constants.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/set_ul.h"
#include "../../src/import/snap_importer.h"
#include "../../src/query/degree.h"

void
test_id_diff_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets)
{
    if (!db || db->node_id_counter < 1 || !dif_sets) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    size_t        num_steps = get_num_steps(db);
    unsigned long num_walks = get_num_walks(db);

    unsigned long*   key      = NULL;
    unsigned long*   value_a  = NULL;
    unsigned long    step_sum = 0;
    htable_t*        ht;
    htable_bucket_t* cur;
    htable_bucket_t* next;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dict_ul_ul_iterator_t* it = create_dict_ul_ul_iterator(dif_sets[i]);

        while (dict_ul_ul_iterator_next(it, &key, &value_a) == 0) {
            step_sum += *value_a;
        }
        dict_ul_ul_iterator_destroy(it);

        if (step_sum < num_steps + 1) {
            printf("i %lu, dict size: %lu Step sum: %lu, n steps %lu \n",
                   i,
                   dict_ul_ul_size(dif_sets[i]),
                   step_sum,
                   num_steps);
            dict_ul_ul_iterator_t* it = create_dict_ul_ul_iterator(dif_sets[i]);

            while (dict_ul_ul_iterator_next(it, &key, &value_a) == 0) {
                printf("Start %lu, Node: %lu, Visited: %lu\n",
                       i,
                       *key,
                       *value_a);
            }
            dict_ul_ul_iterator_destroy(it);

            ht = (htable_t*)dif_sets[i];

            for (size_t j = 0; j < ht->num_buckets; ++j) {
                if (!ht->buckets[j].key) {
                    continue;
                }
                next = ht->buckets[j].next;
                printf("index %lu key %lu, value %lu, is chain 0, has chain "
                       "%d\n",
                       j,
                       *(unsigned long*)ht->buckets[j].key,
                       *(unsigned long*)ht->buckets[j].value,
                       next ? 1 : 0);

                while (next) {
                    cur  = next;
                    next = cur->next;
                    printf("index %lu key %lu, value %lu, is chain 0, has "
                           "chain %d\n",
                           j,
                           *(unsigned long*)cur->key,
                           *(unsigned long*)cur->value,
                           next ? 1 : 0);
                }
            }
            exit(-1);
        }
        assert(step_sum == (num_steps + 1) * num_walks);
        step_sum = 0;
    }
}

void
test_weighted_jaccard_distance(in_memory_file_t* db, dict_ul_ul_t** dif_sets)
{
    if (!db || db->node_id_counter < 1 || !dif_sets) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long* key   = NULL;
    unsigned long* value = NULL;
    unsigned long  value_b;
    unsigned long  union_sum     = 0;
    unsigned long  intersect_sum = 0;

    set_ul_t*              seen_keys_b = create_set_ul();
    dict_ul_ul_iterator_t* it = create_dict_ul_ul_iterator(dif_sets[0]);

    while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
        printf("i 0 key %lu, value %lu\n", *key, *value);
        if (dict_ul_ul_contains(dif_sets[1], *key)) {
            set_ul_insert(seen_keys_b, *key);
            value_b = dict_ul_ul_get_direct(dif_sets[1], *key);

            intersect_sum += *value > value_b ? value_b : *value;
            union_sum += *value > value_b ? *value : value_b;
        } else {
            union_sum += *value;
        }
    }
    dict_ul_ul_iterator_destroy(it);

    it = create_dict_ul_ul_iterator(dif_sets[1]);
    while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
        printf("i 1 key %lu, value %lu\n", *key, *value);
        if (!set_ul_contains(seen_keys_b, *key)) {
            union_sum += *value;
        }
    }
    dict_ul_ul_iterator_destroy(it);
    set_ul_destroy(seen_keys_b);

    assert((1 - ((float)intersect_sum / (float)union_sum))
           == weighted_jaccard_dist(dif_sets[0], dif_sets[1]));

    assert(weighted_jaccard_dist(dif_sets[0], dif_sets[1])
           == weighted_jaccard_dist(dif_sets[1], dif_sets[0]));

    printf("Jaccard_dist %.5f\n",
           weighted_jaccard_dist(dif_sets[0], dif_sets[1]));
}

void
test_insert_match(void)
{
    unsigned long  num_clusters = 2;
    unsigned long* max_degree_nodes =
          malloc(num_clusters * sizeof(unsigned long));
    unsigned long* max_degrees = malloc(num_clusters * sizeof(unsigned long));

    if (!max_degree_nodes || !max_degrees) {
        printf("icbl insert match test: malloc failed\n");
        exit(-1);
    }

    const unsigned long node_id           = 777;
    const unsigned long degree            = 12;
    const unsigned long smaller_degree    = 11;
    const unsigned long larger_id         = 3;
    const unsigned long to_be_replaced_id = 666;
    const unsigned long larger_degree     = 13;

    max_degree_nodes[0] = larger_id;
    max_degree_nodes[1] = to_be_replaced_id;
    max_degrees[0]      = larger_degree;
    max_degrees[1]      = smaller_degree;

    insert_match(max_degree_nodes, max_degrees, node_id, degree, num_clusters);
    assert(max_degrees[1] == degree);
    assert(max_degree_nodes[1] == node_id);
    assert(max_degree_nodes[0] == larger_id);
    assert(max_degrees[0] == larger_degree);

    num_clusters = node_id;
    max_degree_nodes =
          realloc(max_degree_nodes, num_clusters * sizeof(unsigned long));

    if (!max_degree_nodes) {
        free(max_degree_nodes);
        printf("icbl insert match test: malloc failed\n");
        exit(-1);
    }

    max_degrees = realloc(max_degrees, num_clusters * sizeof(unsigned long));
    if (!max_degrees) {
        free(max_degrees);
        printf("icbl insert match test: malloc failed\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        max_degree_nodes[i] = i;
        max_degrees[i]      = num_clusters - i;
    }

    insert_match(max_degree_nodes, max_degrees, node_id, degree, num_clusters);

    assert(max_degree_nodes[766] == node_id);
    assert(max_degrees[766] == degree);

    free(max_degree_nodes);
    free(max_degrees);
}

void
test_check_dist_bound(in_memory_file_t* db, dict_ul_ul_t** dif_sets)
{
    if (!db || db->node_id_counter < 1 || !dif_sets) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    const size_t   num_found        = 7;
    unsigned long* max_degree_nodes = calloc(num_found, sizeof(unsigned long));
    float*         dists            = calloc(num_found, sizeof(float));
    const unsigned long candidate   = 99;

    if (!max_degree_nodes || !dists) {
        printf("icbl insert match test: malloc failed\n");
        exit(-1);
    }

    size_t index = num_found;
    for (size_t i = 0; i < num_found; ++i) {
        max_degree_nodes[i] = i;
        dists[i] = weighted_jaccard_dist(dif_sets[i], dif_sets[candidate]);
        if (dists[i] < MIN_DIST_INIT_CENTERS) {
            index = i;
            break;
        }
    }

    bool ret_val;
    for (size_t i = 0; i < num_found; ++i) {
        ret_val = check_dist_bound(max_degree_nodes, candidate, i, dif_sets);

        if (i < index) {
            assert(ret_val == true);
        } else {
            assert(ret_val == false);
        }
    }

    free(max_degree_nodes);
    free(dists);
}

void
test_initialize_centers(in_memory_file_t* db,
                        dict_ul_ul_t**    dif_sets,
                        unsigned long*    centers,
                        size_t            num_clusters)
{
    if (!db || db->node_id_counter < 1 || !dif_sets || !centers) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    float* dists = calloc(num_clusters * num_clusters, sizeof(float));

    if (!dists) {
        printf("icbl insert match test: malloc failed\n");
        exit(-1);
    }

    printf("avg degree: %.3f\n", get_avg_degree(db, BOTH));

    list_relationship_t* rels;
    for (size_t i = 0; i < num_clusters; ++i) {
        assert(centers[i] != UNINITIALIZED_LONG);
        rels = in_memory_expand(db, centers[i], BOTH);
        printf("center degree i: %lu\n", list_relationship_size(rels));
        list_relationship_destroy(rels);
        for (size_t j = 0; j < i; ++j) {
            if (i == j) {
                continue;
            }
            assert(MIN_DIST_INIT_CENTERS < weighted_jaccard_dist(
                         dif_sets[centers[i]], dif_sets[centers[j]]));
        }
    }

    free(dists);
}

void
test_assign_to_cluster(in_memory_file_t* db,
                       dict_ul_ul_t**    dif_sets,
                       unsigned long*    centers,
                       unsigned long*    part,
                       unsigned long     num_clusters)
{
    if (!db || db->node_id_counter < 1 || !dif_sets || !centers || !part) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long* min_dist_center =
          calloc(db->node_id_counter, sizeof(unsigned long));
    float min_dist = FLT_MAX;
    float dist;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        for (size_t j = 0; j < num_clusters; ++j) {
            dist = weighted_jaccard_dist(dif_sets[i], dif_sets[centers[j]]);
            if (dist < min_dist) {
                min_dist           = dist;
                min_dist_center[i] = j;
            }
        }
        assert(min_dist_center[i] == part[i]);
        min_dist = FLT_MAX;
    }

    assert(assign_to_cluster(
                 db->node_id_counter, dif_sets, part, centers, num_clusters)
           == 0);

    free(min_dist_center);
}

void
test_update_centers(in_memory_file_t* db,
                    dict_ul_ul_t**    dif_sets,
                    unsigned long*    part,
                    unsigned long*    centers,
                    size_t            num_clusters)
{
    if (!db || !dif_sets || !part || !centers || num_clusters < 1) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    unsigned long* centers_copy = malloc(num_clusters * sizeof(unsigned long));

    if (!centers_copy) {
        printf("ICBL test: Memory Allocation failed\n");
        exit(-1);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        centers_copy[i] = centers[i];
    }

    update_centers(db->node_id_counter, dif_sets, part, centers, num_clusters);

    dict_ul_ul_t* cluster_counts[num_clusters];

    for (size_t i = 0; i < num_clusters; ++i) {
        cluster_counts[i] = create_dict_ul_ul();
    }

    dict_ul_ul_iterator_t* it;
    unsigned long*         key   = NULL;
    unsigned long*         value = NULL;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        it = create_dict_ul_ul_iterator(dif_sets[i]);

        while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
            if (dict_ul_ul_contains(cluster_counts[part[i]], *key)) {
                dict_ul_ul_insert(
                      cluster_counts[part[i]],
                      *key,
                      dict_ul_ul_get_direct(cluster_counts[part[i]], *key)
                            + *value);
            } else {
                dict_ul_ul_insert(cluster_counts[part[i]], *key, *value);
            }
        }
        dict_ul_ul_iterator_destroy(it);
    }

    size_t max_idx   = UNINITIALIZED_LONG;
    size_t max_count = 0;
    for (size_t i = 0; i < num_clusters; ++i) {
        it = create_dict_ul_ul_iterator(cluster_counts[i]);

        while (dict_ul_ul_iterator_next(it, &key, &value) == 0) {
            if (*value > max_count) {
                max_count = *value;
                max_idx   = *key;
            }
        }
        centers_copy[i] = max_idx;
        max_count       = 0;
        max_idx         = UNINITIALIZED_LONG;

        dict_ul_ul_iterator_destroy(it);
        dict_ul_ul_destroy(cluster_counts[i]);
    }

    for (size_t i = 0; i < num_clusters; ++i) {
        assert(centers[i] == centers_copy[i]);
    }
    free(centers_copy);
}

int
main(void)
{
    printf("Start importing\n");
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");
    dict_ul_ul_destroy(map);

    if (!db || db->node_id_counter < 1) {
        printf("ICBL test: Invalid Arguments!\n");
        exit(-1);
    }

    dict_ul_ul_t** dif_sets =
          malloc(db->node_id_counter * sizeof(dict_ul_ul_t*));

    if (!dif_sets) {
        printf("ICBL test: Allocating memory failed!\n");
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dif_sets[i] = create_dict_ul_ul();
    }

    const size_t   num_clusters = 7;
    unsigned long* centers;

    unsigned long* part = calloc(db->node_id_counter, sizeof(unsigned long));

    identify_diffustion_sets(db, dif_sets);

    test_id_diff_sets(db, dif_sets);
    test_weighted_jaccard_distance(db, dif_sets);
    test_insert_match();
    test_check_dist_bound(db, dif_sets);

    initialize_centers(db, &centers, num_clusters, dif_sets);

    test_initialize_centers(db, dif_sets, centers, num_clusters);

    size_t changes = assign_to_cluster(
          db->node_id_counter, dif_sets, part, centers, num_clusters);
    printf("changes %lu\n", changes);
    assert(changes > 0);

    test_assign_to_cluster(db, dif_sets, centers, part, num_clusters);
    test_update_centers(db, dif_sets, part, centers, num_clusters);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        dict_ul_ul_destroy(dif_sets[i]);
    }

    free(dif_sets);
    free(centers);
    free(part);

    printf("Start applying the ICBL layout algorithm.\n");
    unsigned long* partition = icbl(db);
    printf("Done.\n");

    FILE* out_f =
          fopen("/home/someusername/workspace_local/icbl_layout_.txt", "w");

    if (!out_f) {
        printf("Couldn't open file");
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        fprintf(out_f, "%lu %lu\n", i, partition[i]);
    }

    fclose(out_f);
    free(partition);

    in_memory_file_destroy(db);
    return 0;
}
