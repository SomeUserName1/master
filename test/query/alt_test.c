#include "../../src/query/alt.h"

#include <assert.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/list_ul.h"
#include "../../src/import/snap_importer.h"
#include "../../src/query/result_types.h"

#define n(x) dict_ul_ul_get_direct(map, x)

int
main(void)
{
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");

    const unsigned long num_landmarks = 3;
    const char* log_path = "/home/someusername/workspace_local/alt_test.txt";

    double** heuristic = malloc(num_landmarks * sizeof(*heuristic));

    alt_preprocess(db, BOTH, num_landmarks, heuristic, log_path);

    path* result =
          alt(db, heuristic, num_landmarks, n(11), n(111), BOTH, log_path);

    assert(result->source == n(11));
    assert(result->target == n(111));
    assert(result->distance == 3);

    assert(list_ul_get(result->edges, 0) == 88);
    assert(list_ul_get(result->edges, 1) == 561);
    assert(list_ul_get(result->edges, 2) == 763);

    path_destroy(result);
    dict_ul_ul_destroy(map);

    for (size_t i = 0; i < num_landmarks; ++i) {
        free(heuristic[i]);
    }

    free(heuristic);
    in_memory_file_destroy(db);
}
