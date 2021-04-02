#include "../../src/query/random_walk.h"

#include "../../src/access/in_memory_file.h"
#include "../../src/import/snap_importer.h"

#include <assert.h>
#include <stdio.h>

int
main(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt"));

    const unsigned int max_walk_steps = 100;
    path* rand_w;
    relationship_t* r;
    relationship_t* r_next;

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(db, 0, i, BOTH);
        assert(rand_w->distance == i);
        assert(rand_w->source == 0);
        assert(list_ul_size(rand_w->edges) == i);

        for (size_t j = 0; j < i - 1; ++j) {
            // for direction BOTH, consecutive edges need to share one node, no
            // matter if src or target
            r = in_memory_get_relationship(db, list_ul_get(rand_w->edges, j));
            r_next = in_memory_get_relationship(
                  db, list_ul_get(rand_w->edges, j + 1));
            assert(r->target_node == r_next->target_node ||
                   r->source_node == r_next->source_node ||
                   r->source_node == r_next->target_node ||
                   r->target_node == r_next->source_node);
        }

        path_destroy(rand_w);
    }

    // The asserts below use <= instead of == as a random walk can end up with
    // no edges to take for directed random walks.

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(db, 0, i, OUTGOING);
        assert(rand_w->distance <= i);
        assert(rand_w->source == 0);
        assert(list_ul_size(rand_w->edges) <= i);

        for (size_t j = 0; j < rand_w->distance - 1; ++j) {
            // for direction OUTGOING, r target must correspond to r_next's
            // source
            r = in_memory_get_relationship(db, list_ul_get(rand_w->edges, j));
            r_next = in_memory_get_relationship(
                  db, list_ul_get(rand_w->edges, j + 1));
            assert(r->target_node == r_next->source_node);
        }

        path_destroy(rand_w);
    }

    for (size_t i = 1; i < max_walk_steps; ++i) {
        rand_w = random_walk(db, 0, i, INCOMING);
        assert(rand_w->distance <= i);
        assert(rand_w->source == 0);
        assert(list_ul_size(rand_w->edges) <= i);

        for (size_t j = 0; j < rand_w->distance - 1; ++j) {
            // for direction OUTGOING, r target must correspond to r_next's
            // source
            r = in_memory_get_relationship(db, list_ul_get(rand_w->edges, j));
            r_next = in_memory_get_relationship(
                  db, list_ul_get(rand_w->edges, j + 1));
            assert(r->source_node == r_next->target_node);
        }

        path_destroy(rand_w);
    }

    in_memory_file_destroy(db);
}
