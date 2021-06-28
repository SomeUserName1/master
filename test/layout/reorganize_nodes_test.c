#include "layout/reorganize_nodes.h"

#include <assert.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "query/louvain.h"
#include "query/operators.h"
#include "query/snap_importer.h"

void
test_remap_node_ids(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* partition = louvain(db);

    array_list_node* old_ordered_nodes = in_memory_get_nodes(db);
    unsigned long*   old_order =
          calloc(db->node_id_counter, sizeof(unsigned long));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        old_order[i] = array_list_node_get(old_ordered_nodes, i)->id;
    }
    array_list_node_destroy(old_ordered_nodes);

    unsigned long* id_map = remap_node_ids(db, partition);

    array_list_ul* part_numbers = al_ul_create();
    array_list_ul_append(part_numbers, partition[0]);
    // Count the number of partitions
    for (size_t i = 1; i < db->node_id_counter; ++i) {
        if (!array_list_ul_contains(part_numbers, partition[i])) {
            array_list_ul_append(part_numbers, partition[i]);
        }
    }

    size_t num_parts = array_list_ul_size(part_numbers);
    array_list_ul_destroy(part_numbers);

    array_list_ul** nodes_per_partition =
          calloc(num_parts, sizeof(array_list_ul*));

    for (size_t i = 0; i < num_parts; ++i) {
        nodes_per_partition[i] = al_ul_create();
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        array_list_ul_append(nodes_per_partition[partition[i]], i);
    }

    unsigned long* max_id_per_part = calloc(num_parts, sizeof(unsigned long));

    max_id_per_part[0] = array_list_ul_size(nodes_per_partition[0]) - 1;

    for (size_t i = 1; i < num_parts; ++i) {
        max_id_per_part[i] = max_id_per_part[i - 1]
                             + array_list_ul_size(nodes_per_partition[i]);
    }

    for (size_t i = 0; i < num_parts; ++i) {
        array_list_ul_destroy(nodes_per_partition[i]);
    }
    free(nodes_per_partition);

    array_list_ul* seen_ids = al_ul_create();
    node_t*        node;
    unsigned long  min_node_id;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        node = in_memory_get_node(db, id_map[old_order[i]]);

        assert(!array_list_ul_contains(seen_ids, node->id));
        min_node_id =
              partition[i] == 0 ? 0 : max_id_per_part[partition[i] - 1] + 1;
        assert(node->id >= min_node_id);
        assert(node->id <= max_id_per_part[partition[i]]);
        array_list_ul_append(seen_ids, node->id);
    }
    free(old_order);
    free(id_map);
    free(max_id_per_part);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        assert(array_list_ul_contains(seen_ids, i));
    }

    array_list_relationship* rels;
    relationship_t*          rel;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        node = in_memory_get_node(db, i);
        rels = in_memory_expand(db, node->id, BOTH);

        for (size_t j = 0; j < array_list_relationship_size(rels); ++j) {
            rel = array_list_relationship_get(rels, j);
            assert(rel->source_node == node->id
                   || rel->target_node == node->id);
        }

        array_list_relationship_destroy(rels);
    }

    in_memory_file_destroy(db);
    array_list_ul_destroy(seen_ids);
    free(partition);
}

int
main(void)
{
    test_remap_node_ids();
}
