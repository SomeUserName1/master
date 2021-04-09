#include "../../src/query/dfs.h"

#include <assert.h>
#include <stdio.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/query/result_types.h"

#define NUM_NODES (10)
static const unsigned long node0 = 0;
static const unsigned long node1 = 1;
static const unsigned long node2 = 2;
static const unsigned long node3 = 3;
static const unsigned long node4 = 4;
static const unsigned long node5 = 5;
static const unsigned long node6 = 6;
static const unsigned long node7 = 7;
static const unsigned long node8 = 8;
static const unsigned long node9 = 9;

int
main(void)
{
    in_memory_file_t* db = create_in_memory_file();

    for (size_t i = 0; i < NUM_NODES; ++i) {
        in_memory_create_node(db);
    }

    in_memory_create_relationship(db, node0, node1);
    in_memory_create_relationship(db, node0, node2);
    in_memory_create_relationship(db, node0, node3);

    in_memory_create_relationship(db, node1, node4);
    in_memory_create_relationship(db, node1, node5);

    in_memory_create_relationship(db, node2, node6);
    in_memory_create_relationship(db, node2, node7);

    in_memory_create_relationship(db, node3, node8);
    in_memory_create_relationship(db, node3, node9);

    traversal_result* result =
          dfs(db, 0, BOTH, "/home/someusername/workspace_local/dfs_test.txt");

    assert(result->source == 0);
    assert(result->traversal_numbers[0] == 0);

    assert(result->traversal_numbers[1] == 1);
    assert(result->parents[1] == 0);

    assert(result->traversal_numbers[2] == 1);
    assert(result->parents[2] == 1);

    assert(result->traversal_numbers[3] == 1);
    assert(result->parents[3] == 2);

    assert(result->traversal_numbers[8] == 2);
    assert(result->parents[8] == 7);

    assert(result->traversal_numbers[9] == 2);
    assert(result->parents[9] == 8);

    assert(result->traversal_numbers[6] == 2);
    assert(result->parents[6] == 5);

    assert(result->traversal_numbers[7] == 2);
    assert(result->parents[7] == 6);

    assert(result->traversal_numbers[4] == 2);
    assert(result->parents[4] == 3);

    assert(result->traversal_numbers[5] == 2);
    assert(result->parents[5] == 4);

    traversal_result_destroy(result);
    in_memory_file_destroy(db);
}
