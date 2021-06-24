#include "query/a-star.h"

#include <assert.h>
#include <stdlib.h>

#include "access/operators.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "query/result_types.h"
#include "query/snap_importer.h"

#define n(x) dict_ul_ul_get_direct(map, x)

int
main(void)
{
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul*       map = import_from_txt(
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

    assert(array_list_ul_get(result->edges, 0) == 88);
    assert(array_list_ul_get(result->edges, 1) == 561);
    assert(array_list_ul_get(result->edges, 2) == 763);

    path_destroy(result);
    dict_ul_ul_destroy(map);
    free(heuristic);
    in_memory_file_destroy(db);
}
