#include <stdio.h>

#include "access/in_memory_file.h"
#include "data-struct/list_ul.h"
#include "import/snap_importer.h"
#include "query/bfs.h"

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
    printf("Start a BFS\n");
    list_ul_t* path = bfs(db, 0, 999);

    printf("Shortest path:");
    for (size_t i = 0; i < list_ul_size(path); i++) {
        printf(" %lu", list_ul_get(path, i));
    }

    printf("\nSuccess!\n");
    list_ul_destroy(path);
    in_memory_file_destroy(db);
    return 0;
}
