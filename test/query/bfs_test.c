#include "../../src/query/bfs.h"

#include "../../src/access/in_memory_file.h"
#include "../../src/import/snap_importer.h"

#include <assert.h>
#include <stdio.h>

#define n(x) dict_ul_ul_get_direct(map, x)

int
main(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(
          db, "/home/someusername/workspace_local/celegans.txt");

    traversal_result* result = bfs(
          db, n(11), BOTH, "/home/someusername/workspace_local/bfs_test.txt");

    assert(result->source == n(11));
    assert(result->traversal_numbers[n(11)] == 0);

    assert(result->traversal_numbers[n(0)] == 1);
    assert(result->parents[n(0)] == 0);

    assert(result->traversal_numbers[n(6)] == 1);
    assert(result->parents[n(6)] == 48);

    assert(result->traversal_numbers[n(8)] == 1);
    assert(result->parents[n(8)] == 83);

    assert(result->traversal_numbers[n(14)] == 1);
    assert(result->parents[n(14)] == 84);

    assert(result->traversal_numbers[n(20)] == 1);
    assert(result->parents[n(20)] == 85);

    assert(result->traversal_numbers[n(35)] == 1);
    assert(result->parents[n(35)] == 86);

    assert(result->traversal_numbers[n(71)] == 1);
    assert(result->parents[n(71)] == 87);

    assert(result->traversal_numbers[n(78)] == 1);
    assert(result->parents[n(78)] == 88);

    assert(result->traversal_numbers[n(79)] == 1);
    assert(result->parents[n(79)] == 89);

    assert(result->traversal_numbers[n(112)] == 1);
    assert(result->parents[n(112)] == 90);

    assert(result->traversal_numbers[n(114)] == 1);
    assert(result->parents[n(114)] == 91);

    assert(result->traversal_numbers[n(41)] == 1);
    assert(result->parents[n(41)] == 234);

    assert(result->traversal_numbers[n(43)] == 1);
    assert(result->parents[n(43)] == 247);

    assert(result->traversal_numbers[n(83)] == 1);
    assert(result->parents[n(83)] == 587);

    assert(result->traversal_numbers[n(26)] == 2);
    assert(result->parents[n(26)] == 1);

    assert(result->traversal_numbers[n(69)] == 2);
    assert(result->parents[n(69)] == 2);

    assert(result->traversal_numbers[n(18)] == 2);
    assert(result->parents[n(18)] == 122);

    assert(result->traversal_numbers[n(2)] == 2);
    assert(result->parents[n(2)] == 15);

    assert(result->traversal_numbers[n(15)] == 2);
    assert(result->parents[n(15)] == 49);

    assert(result->traversal_numbers[n(16)] == 2);
    assert(result->parents[n(16)] == 50);

    assert(result->traversal_numbers[n(24)] == 2);
    assert(result->parents[n(24)] == 51);

    assert(result->traversal_numbers[n(46)] == 2);
    assert(result->parents[n(46)] == 52);

    assert(result->traversal_numbers[n(10)] == 2);
    assert(result->parents[n(10)] == 76);

    assert(result->traversal_numbers[n(45)] == 2);
    assert(result->parents[n(45)] == 258);

    assert(result->traversal_numbers[n(3)] == 2);
    assert(result->parents[n(3)] == 32);

    assert(result->traversal_numbers[n(7)] == 2);
    assert(result->parents[n(7)] == 54);

    assert(result->traversal_numbers[n(29)] == 2);
    assert(result->parents[n(29)] == 59);

    assert(result->traversal_numbers[n(31)] == 2);
    assert(result->parents[n(31)] == 60);

    assert(result->traversal_numbers[n(34)] == 2);
    assert(result->parents[n(34)] == 61);

    assert(result->traversal_numbers[n(73)] == 2);
    assert(result->parents[n(73)] == 63);

    assert(result->traversal_numbers[n(80)] == 2);
    assert(result->parents[n(80)] == 65);

    assert(result->traversal_numbers[n(100)] == 2);
    assert(result->parents[n(100)] == 66);

    assert(result->traversal_numbers[n(116)] == 2);
    assert(result->parents[n(116)] == 67);

    assert(result->traversal_numbers[n(12)] == 2);
    assert(result->parents[n(12)] == 93);

    assert(result->traversal_numbers[n(17)] == 2);
    assert(result->parents[n(17)] == 121);

    assert(result->traversal_numbers[n(19)] == 2);
    assert(result->parents[n(19)] == 135);

    assert(result->traversal_numbers[n(25)] == 2);
    assert(result->parents[n(25)] == 158);

    assert(result->traversal_numbers[n(97)] == 2);
    assert(result->parents[n(97)] == 635);

    assert(result->traversal_numbers[n(101)] == 2);
    assert(result->parents[n(101)] == 642);

    assert(result->traversal_numbers[n(9)] == 2);
    assert(result->parents[n(9)] == 69);

    assert(result->traversal_numbers[n(1)] == 2);
    assert(result->parents[n(1)] == 103);

    assert(result->traversal_numbers[n(72)] == 2);
    assert(result->parents[n(72)] == 108);

    assert(result->traversal_numbers[n(21)] == 2);
    assert(result->parents[n(21)] == 149);

    assert(result->traversal_numbers[n(42)] == 2);
    assert(result->parents[n(42)] == 241);

    assert(result->traversal_numbers[n(13)] == 2);
    assert(result->parents[n(13)] == 101);

    assert(result->traversal_numbers[n(27)] == 2);
    assert(result->parents[n(27)] == 166);

    assert(result->traversal_numbers[n(30)] == 2);
    assert(result->parents[n(30)] == 182);

    assert(result->traversal_numbers[n(33)] == 2);
    assert(result->parents[n(33)] == 198);

    assert(result->traversal_numbers[n(38)] == 2);
    assert(result->parents[n(38)] == 223);

    assert(result->traversal_numbers[n(40)] == 2);
    assert(result->parents[n(40)] == 232);

    assert(result->traversal_numbers[n(47)] == 2);
    assert(result->parents[n(47)] == 269);

    assert(result->traversal_numbers[n(49)] == 2);
    assert(result->parents[n(49)] == 278);

    traversal_result_destroy(result);
    dict_ul_ul_destroy(map);
    in_memory_file_destroy(db);
}
