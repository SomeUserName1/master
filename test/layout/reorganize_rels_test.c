#include "layout/reorganize_relationships.h"

#include <assert.h>
#include <stdlib.h>

#include "access/in_memory_file.h"
#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "query/in_memory_operators.h"
#include "query/snap_importer.h"

void
test_remap_rel_ids(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* degrees = calloc(db->node_id_counter, sizeof(unsigned long));
    array_list_ul** incidence_array_lists =
          calloc(db->node_id_counter, sizeof(array_list_ul*));

    array_list_relationship* rels;
    relationship_t*          rel;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels                     = in_memory_expand(db, i, BOTH);
        incidence_array_lists[i] = al_ul_create();

        degrees[i] = array_list_relationship_size(rels);
        for (size_t j = 0; j < degrees[i]; ++j) {
            array_list_ul_append(incidence_array_lists[i],
                                 array_list_relationship_get(rels, j)->id);
        }
        array_list_relationship_destroy(rels);
    }

    unsigned long* id_map = remap_rel_ids(db);

    unsigned long  prev_rel_id = 0;
    unsigned long  rel_id;
    array_list_ul* seen_ids = al_ul_create();

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, OUTGOING);

        if (array_list_relationship_size(rels) > 0) {
            prev_rel_id = array_list_relationship_get(rels, 0)->id;
            array_list_ul_append(seen_ids, prev_rel_id);
        }

        for (size_t j = 1; j < array_list_relationship_size(rels); ++j) {
            rel_id = array_list_relationship_get(rels, j)->id;
            assert(rel_id - 1 == prev_rel_id);
            assert(!array_list_ul_contains(seen_ids, rel_id));
            array_list_ul_append(seen_ids, rel_id);
            prev_rel_id++;
        }
        array_list_relationship_destroy(rels);
    }

    for (size_t i = 0; i < db->rel_id_counter; ++i) {
        assert(array_list_ul_contains(seen_ids, i));
    }
    array_list_ul_destroy(seen_ids);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);

        assert(degrees[i] == array_list_relationship_size(rels));

        for (size_t j = 0; j < degrees[i]; ++j) {
            rel = array_list_relationship_get(rels, j);
            if ((rel->source_node == i && rel->flags & FIRST_REL_SOURCE_FLAG)
                || (rel->target_node == i
                    && rel->flags & FIRST_REL_TARGET_FLAG)) {

                assert(in_memory_get_node(db, i)->first_relationship
                       == rel->id);
            }
            assert(id_map[array_list_ul_get(incidence_array_lists[i], j)]
                   == array_list_relationship_get(rels, j)->id);
        }
        array_list_relationship_destroy(rels);
        array_list_ul_destroy(incidence_array_lists[i]);
    }
    free(incidence_array_lists);
    free(id_map);
    free(degrees);
    in_memory_file_destroy(db);
}

void
test_sort_incidence_array_list(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    unsigned long* degrees = calloc(db->node_id_counter, sizeof(unsigned long));
    array_list_ul** incidence_array_lists =
          calloc(db->node_id_counter, sizeof(array_list_ul*));

    array_list_relationship* rels;

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels                     = in_memory_expand(db, i, BOTH);
        incidence_array_lists[i] = al_ul_create();

        degrees[i] = array_list_relationship_size(rels);
        for (size_t j = 0; j < degrees[i]; ++j) {
            array_list_ul_append(incidence_array_lists[i],
                                 array_list_relationship_get(rels, j)->id);
        }
        array_list_relationship_destroy(rels);
    }

    sort_incidence_list(db);

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        rels = in_memory_expand(db, i, BOTH);

        for (size_t j = 0; j < degrees[i]; ++j) {
            assert(array_list_ul_contains(
                  incidence_array_lists[i],
                  array_list_relationship_get(rels, j)->id));
        }
        array_list_relationship_destroy(rels);
        array_list_ul_destroy(incidence_array_lists[i]);
    }
    free(incidence_array_lists);
    free(degrees);
    in_memory_file_destroy(db);
}

int
main(void)
{
    test_remap_rel_ids();
    test_sort_incidence_array_list();
}
