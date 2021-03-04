#include "icbp.h"

#include <stdlib.h>
#include <math.h>

#include "../../query/degree.h"
#include "../../data-struct/list_ul.h"
#include "../../query/random_walk.h"

size_t get_num_walks(in_memory_file_t* db) {
    size_t min_deg = get_min_degree(db, BOTH);
    size_t max_deg = get_max_degree(db, BOTH);
    size_t range = max_deg - min_deg + 1;
    size_t nodes_per_step = db->node_id_counter / range;

    list_node_t* nodes = in_memory_get_nodes(db);
    list_relationship_t* rels;

    size_t* degree_hist = calloc(range, sizeof(size_t));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);
        degree_hist[min_deg - list_relationship_size(rels)]++;
    }

    for (size_t i = 0; i < range; ++i) {
        if (degree_hist[i] > nodes_per_step) {
            return i;
        }
    }

    return max_deg;
}

size_t get_num_coarse_clusters(in_memory_file_t* db) {
    return (sizeof(node_t) * db->node_id_counter) / sqrt(0.8 * MEMORY);
}

size_t get_num_steps(in_memory_file_t* db) {
    return 1 + ceil(log2((float) db->node_id_counter) / get_num_coarse_clusters(db));
}

int gen_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets) {
    list_node_t* nodes = in_memory_get_nodes(db);
    size_t num_nodes = list_node_size(nodes);
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


int cluster_coarse(in_memory_file_t* db, dict_ul_ul_t** dif_sets, size_t* parts) {

}
