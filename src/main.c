#include <stdio.h>
#include <stdlib.h>

#include "access/in_memory_file.h"
#include "constants.h"
#include "data-struct/dict_ul.h"
#include "import/snap_importer.h"
#include "locality/ids_to_io.h"
#include "query/bfs.h"
#include "query/louvain.h"
#include "query/result_types.h"

int
main(void)
{
    printf("Start importing\n");
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/email_eu.txt");
    dict_ul_ul_destroy(map);
    printf("BFS\n");
    traversal_result* result =
          bfs(db,
              0,
              BOTH,
              "/home/someusername/workspace_local/accessed_records.txt");

    printf("Analyze IOs from IDs\n");
    io_stats_t* res = ids_to_io(
          "/home/someusername/workspace_local/accessed_records.txt",
          "/home/someusername/workspace_local/block_access_sequence.txt",
          BLOCK_SIZE,
          PAGE_SIZE,
          REL);
    if (res == NULL) {
        return -1;
    }
    printf("%s %lu %s %lu %s",
           "Loaded",
           res->read_pages,
           "Pages and accessed thereby",
           res->read_blocks,
           "Blocks.\n");

    unsigned long* partition = louvain(db);
    printf("%p", partition);
    // block_order = partition_to_block_order(db, partition);
    // reorder_storage(db, block_order, partiton);
    free(partition);

    printf("\nSuccess!\n");
    traversal_result_destroy(result);
    free(res);
    in_memory_file_destroy(db);
    return 0;
}
