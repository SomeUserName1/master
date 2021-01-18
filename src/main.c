#include <stdio.h>
#include <stdlib.h>

#include "access/in_memory_file.h"
#include "import/snap_importer.h"
#include "query/bfs.h"
#include "analyze/ids_to_io.h"

int main(void) {
   dataset_t dataset = EMAIL_EU_CORE;

    printf("Start downloading\n");
    if (download_dataset(dataset, "/home/someusername/workspace_local/dataset.txt.gz") < 0) {
        printf("Downloading the EMAIL EU CORE dataset failed\n");
        return -1;
    }
    printf("start uncompressing\n");
    if (uncompress_dataset("/home/someusername/workspace_local/dataset.txt.gz", "/home/someusername/workspace_local/email_eu.txt") < 0) {
        printf("Uncompressing failed\n");
        return -1;
    }
    printf("Start importing\n");
    in_memory_file_t* db = create_in_memory_file();
    if (import_from_txt(db, "/home/someusername/workspace_local/email_eu.txt") < 0) {
        printf("Importing failed!\n");
    }
    printf("BFS\n");
    bfs_result_t* result = bfs(db, 0, "/home/someusername/workspace_local/accessed_records.txt");

    printf("Analyze IOs from IDs\n");
    io_stats_t* res = ids_to_io("/home/someusername/workspace_local/accessed_records.txt", 4096, 512, REL);
    if (res == NULL) {
        return -1;
    }
    printf("%s %lu %s %lu %s %d\n", "Loaded", res->read_pages, "Pages and accessed thereby", res->read_blocks, "Blocks.\nAs reads are performed page-wise blocks are loaded sequentially in groups of", 4096 / 512);

    printf("\nSuccess!\n");
    bfs_result_destroy(result);
    free(res);
    in_memory_file_destroy(db);
    return 0;
}
