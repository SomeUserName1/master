#include "../../src/query/a-star.h"

#include "../../src/access/in_memory_file.h"
#include "../../src/import/snap_importer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define n(x) dict_ul_ul_get_direct(map, x)

int
main(void)
{
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");

    double* heuristic = calloc(db->node_id_counter, sizeof(*heuristic));

    path* result = a_star(db,
                          heuristic,
                          n(11),
                          n(111),
                          BOTH,
                          "/home/someusername/workspace_local/a-star_test.txt");

    assert(result->source == n(11));
    assert(result->target == n(111));
    assert(result->distance == 3);

    assert(list_ul_get(result->edges, 0) == 88);
    assert(list_ul_get(result->edges, 1) == 561);
    assert(list_ul_get(result->edges, 2) == 763);

    path_destroy(result);
    dict_ul_ul_destroy(map);
    free(heuristic);
    in_memory_file_destroy(db);
}
