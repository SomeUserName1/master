#include "../../src/locality/reorganize_records.h"

#include <assert.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/constants.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/list_node.h"
#include "../../src/data-struct/list_rel.h"
#include "../../src/data-struct/list_ul.h"
#include "../../src/import/snap_importer.h"
#include "../../src/query/louvain.h"
#include "../../src/record/node.h"
#include "../../src/record/relationship.h"

void
test_remap_node_ids(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* partition = louvain(db);

    // FIXME HEAP used after free: construct map for old enum otherwise
    list_node_t*   old_ordered_nodes = in_memory_get_nodes(db);
    unsigned long* old_order =
          calloc(db->node_id_counter, sizeof(unsigned long));

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        old_order[i] = list_node_get(old_ordered_nodes, i)->id;
    }
    list_node_destroy(old_ordered_nodes);

    unsigned long* id_map = remap_node_ids(db, partition);

    list_ul_t* part_numbers = create_list_ul();
    list_ul_append(part_numbers, partition[0]);
    // Count the number of partitions
    for (size_t i = 1; i < db->node_id_counter; ++i) {
        if (!list_ul_contains(part_numbers, partition[i])) {
            list_ul_append(part_numbers, partition[i]);
        }
    }

    size_t num_parts = list_ul_size(part_numbers);
    list_ul_destroy(part_numbers);

    list_ul_t** nodes_per_partition = calloc(num_parts, sizeof(list_ul_t*));

    for (size_t i = 0; i < num_parts; ++i) {
        nodes_per_partition[i] = create_list_ul();
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        list_ul_append(nodes_per_partition[partition[i]], i);
    }

    unsigned long* max_id_per_part = calloc(num_parts, sizeof(unsigned long));

    max_id_per_part[0] = list_ul_size(nodes_per_partition[0]) - 1;

    for (size_t i = 1; i < num_parts; ++i) {
        max_id_per_part[i] =
              max_id_per_part[i - 1] + list_ul_size(nodes_per_partition[i]);
    }

    for (size_t i = 0; i < num_parts; ++i) {
        list_ul_destroy(nodes_per_partition[i]);
    }
    free(nodes_per_partition);

    list_ul_t*    seen_ids = create_list_ul();
    node_t*       node;
    unsigned long min_node_id;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        node = in_memory_get_node(db, id_map[old_order[i]]);

        assert(!list_ul_contains(seen_ids, node->id));
        min_node_id =
              partition[i] == 0 ? 0 : max_id_per_part[partition[i] - 1] + 1;
        assert(node->id >= min_node_id);
        assert(node->id <= max_id_per_part[partition[i]]);
        list_ul_append(seen_ids, node->id);
    }
    free(old_order);
    free(id_map);
    free(max_id_per_part);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        assert(list_ul_contains(seen_ids, i));
    }

    list_relationship_t* rels;
    relationship_t*      rel;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        node = in_memory_get_node(db, i);
        rels = in_memory_expand(db, node->id, BOTH);

        for (size_t j = 0; j < list_relationship_size(rels); ++j) {
            rel = list_relationship_get(rels, j);
            assert(rel->source_node == node->id
                   || rel->target_node == node->id);
        }

        list_relationship_destroy(rels);
    }

    in_memory_file_destroy(db);
    list_ul_destroy(seen_ids);
    free(partition);
}

void
test_remap_rel_ids(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* degrees = calloc(db->node_id_counter, sizeof(unsigned long));
    list_ul_t**    incidence_lists =
          calloc(db->node_id_counter, sizeof(list_ul_t*));

    list_relationship_t* rels;
    relationship_t*      rel;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels               = in_memory_expand(db, i, BOTH);
        incidence_lists[i] = create_list_ul();

        degrees[i] = list_relationship_size(rels);
        for (size_t j = 0; j < degrees[i]; ++j) {
            list_ul_append(incidence_lists[i],
                           list_relationship_get(rels, j)->id);
        }
        list_relationship_destroy(rels);
    }

    unsigned long* id_map = remap_rel_ids(db);

    unsigned long prev_rel_id = 0;
    unsigned long rel_id;
    list_ul_t*    seen_ids = create_list_ul();

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, OUTGOING);

        if (list_relationship_size(rels) > 0) {
            prev_rel_id = list_relationship_get(rels, 0)->id;
            list_ul_append(seen_ids, prev_rel_id);
        }

        for (size_t j = 1; j < list_relationship_size(rels); ++j) {
            rel_id = list_relationship_get(rels, j)->id;
            assert(rel_id - 1 == prev_rel_id);
            assert(!list_ul_contains(seen_ids, rel_id));
            list_ul_append(seen_ids, rel_id);
            prev_rel_id++;
        }
        list_relationship_destroy(rels);
    }

    for (size_t i = 0; i < db->rel_id_counter; ++i) {
        assert(list_ul_contains(seen_ids, i));
    }
    list_ul_destroy(seen_ids);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);

        assert(degrees[i] == list_relationship_size(rels));

        for (size_t j = 0; j < degrees[i]; ++j) {
            rel = list_relationship_get(rels, j);
            if ((rel->source_node == i && rel->flags & FIRST_REL_SOURCE_FLAG)
                || (rel->target_node == i
                    && rel->flags & FIRST_REL_TARGET_FLAG)) {

                assert(in_memory_get_node(db, i)->first_relationship
                       == rel->id);
            }
            assert(id_map[list_ul_get(incidence_lists[i], j)]
                   == list_relationship_get(rels, j)->id);
        }
        list_relationship_destroy(rels);
        list_ul_destroy(incidence_lists[i]);
    }
    free(incidence_lists);
    free(id_map);
    free(degrees);
    in_memory_file_destroy(db);
}

void
test_sort_incidence_list(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* degrees = calloc(db->node_id_counter, sizeof(unsigned long));
    list_ul_t**    incidence_lists =
          calloc(db->node_id_counter, sizeof(list_ul_t*));

    list_relationship_t* rels;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels               = in_memory_expand(db, i, BOTH);
        incidence_lists[i] = create_list_ul();

        degrees[i] = list_relationship_size(rels);
        for (size_t j = 0; j < degrees[i]; ++j) {
            list_ul_append(incidence_lists[i],
                           list_relationship_get(rels, j)->id);
        }
        list_relationship_destroy(rels);
    }
    sort_incidence_list(db);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);
        assert(degrees[i] == list_relationship_size(rels));

        for (size_t j = 0; j < degrees[i]; ++j) {
            assert(list_ul_contains(incidence_lists[i],
                                    list_relationship_get(rels, j)->id));
        }
        list_relationship_destroy(rels);
        list_ul_destroy(incidence_lists[i]);
    }
    free(incidence_lists);
    free(degrees);
    in_memory_file_destroy(db);
}

void
test_reorganize_records(void)
{}

int
main(void)
{
    test_remap_node_ids();
    test_remap_rel_ids();
    test_sort_incidence_list();
    test_reorganize_records();
}
