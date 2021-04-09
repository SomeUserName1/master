#include "../../src/query/dijkstra.h"

#include <assert.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/import/snap_importer.h"
#include "../../src/query/result_types.h"

#define n(x) dict_ul_ul_get_direct(map, x)

int
main(void)
{
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");

    sssp_result* result =
          dijkstra(db,
                   n(11),
                   BOTH,
                   "/home/someusername/workspace_local/dijkstra_test.txt");

    assert(result->source == n(11));
    assert(result->distances[n(11)] == 0);

    assert(result->distances[n(0)] == 1);
    assert(result->pred_edges[n(0)] == 0);

    assert(result->distances[n(6)] == 1);
    assert(result->pred_edges[n(6)] == 48);

    assert(result->distances[n(8)] == 1);
    assert(result->pred_edges[n(8)] == 83);

    assert(result->distances[n(14)] == 1);
    assert(result->pred_edges[n(14)] == 84);

    assert(result->distances[n(20)] == 1);
    assert(result->pred_edges[n(20)] == 85);

    assert(result->distances[n(35)] == 1);
    assert(result->pred_edges[n(35)] == 86);

    assert(result->distances[n(71)] == 1);
    assert(result->pred_edges[n(71)] == 87);

    assert(result->distances[n(78)] == 1);
    assert(result->pred_edges[n(78)] == 88);

    assert(result->distances[n(79)] == 1);
    assert(result->pred_edges[n(79)] == 89);

    assert(result->distances[n(112)] == 1);
    assert(result->pred_edges[n(112)] == 90);

    assert(result->distances[n(114)] == 1);
    assert(result->pred_edges[n(114)] == 91);

    assert(result->distances[n(41)] == 1);
    assert(result->pred_edges[n(41)] == 234);

    assert(result->distances[n(43)] == 1);
    assert(result->pred_edges[n(43)] == 247);

    assert(result->distances[n(83)] == 1);
    assert(result->pred_edges[n(83)] == 587);

    assert(result->distances[n(26)] == 2);
    assert(result->pred_edges[n(26)] == 1);

    assert(result->distances[n(69)] == 2);
    assert(result->pred_edges[n(69)] == 2);

    assert(result->distances[n(18)] == 2);
    assert(result->pred_edges[n(18)] == 122);

    assert(result->distances[n(2)] == 2);
    assert(result->pred_edges[n(2)] == 15);

    assert(result->distances[n(15)] == 2);
    assert(result->pred_edges[n(15)] == 49);

    assert(result->distances[n(16)] == 2);
    assert(result->pred_edges[n(16)] == 50);

    assert(result->distances[n(24)] == 2);
    assert(result->pred_edges[n(24)] == 51);

    assert(result->distances[n(46)] == 2);
    assert(result->pred_edges[n(46)] == 52);

    assert(result->distances[n(10)] == 2);
    assert(result->pred_edges[n(10)] == 76);

    assert(result->distances[n(45)] == 2);
    assert(result->pred_edges[n(45)] == 258);

    assert(result->distances[n(3)] == 2);
    assert(result->pred_edges[n(3)] == 32);

    assert(result->distances[n(7)] == 2);
    assert(result->pred_edges[n(7)] == 54);

    assert(result->distances[n(29)] == 2);
    assert(result->pred_edges[n(29)] == 59);

    assert(result->distances[n(31)] == 2);
    assert(result->pred_edges[n(31)] == 60);

    assert(result->distances[n(34)] == 2);
    assert(result->pred_edges[n(34)] == 61);

    assert(result->distances[n(73)] == 2);
    assert(result->pred_edges[n(73)] == 63);

    assert(result->distances[n(80)] == 2);
    assert(result->pred_edges[n(80)] == 65);

    assert(result->distances[n(100)] == 2);
    assert(result->pred_edges[n(100)] == 66);

    assert(result->distances[n(116)] == 2);
    assert(result->pred_edges[n(116)] == 67);

    assert(result->distances[n(12)] == 2);
    assert(result->pred_edges[n(12)] == 93);

    assert(result->distances[n(17)] == 2);
    assert(result->pred_edges[n(17)] == 121);

    assert(result->distances[n(19)] == 2);
    assert(result->pred_edges[n(19)] == 135);

    assert(result->distances[n(25)] == 2);
    assert(result->pred_edges[n(25)] == 158);

    assert(result->distances[n(97)] == 2);
    assert(result->pred_edges[n(97)] == 635);

    assert(result->distances[n(101)] == 2);
    assert(result->pred_edges[n(101)] == 642);

    assert(result->distances[n(9)] == 2);
    assert(result->pred_edges[n(9)] == 69);

    assert(result->distances[n(1)] == 2);
    assert(result->pred_edges[n(1)] == 539);

    assert(result->distances[n(72)] == 2);
    assert(result->pred_edges[n(72)] == 479);

    assert(result->distances[n(21)] == 2);
    assert(result->pred_edges[n(21)] == 149);

    assert(result->distances[n(42)] == 2);
    assert(result->pred_edges[n(42)] == 244);

    assert(result->distances[n(13)] == 2);
    assert(result->pred_edges[n(13)] == 101);

    assert(result->distances[n(27)] == 2);
    assert(result->pred_edges[n(27)] == 542);

    assert(result->distances[n(30)] == 2);
    assert(result->pred_edges[n(30)] == 563);

    assert(result->distances[n(33)] == 2);
    assert(result->pred_edges[n(33)] == 198);

    assert(result->distances[n(38)] == 2);
    assert(result->pred_edges[n(38)] == 223);

    assert(result->distances[n(40)] == 2);
    assert(result->pred_edges[n(40)] == 232);

    assert(result->distances[n(47)] == 2);
    assert(result->pred_edges[n(47)] == 269);

    assert(result->distances[n(49)] == 2);
    assert(result->pred_edges[n(49)] == 544);

    sssp_result_destroy(result);
    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}
