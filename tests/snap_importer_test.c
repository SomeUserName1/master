#include "../src/importer/snap_importer.h"
#include <stdio.h>

int main(void) {
    printf("At least the entry point is right\n");
   dataset_t dataset = ORKUT;

    printf("Start downloading\n");
    if (download_dataset(dataset, "/home/someusername/workspace_local/dataset.txt.gz") < 0) {
        printf("Downloading the EMAIL EU CORE dataset failed\n");
        return -1;
    }
    printf("start uncompressing\n");
    if (uncompress_dataset("/home/someusername/workspace_local/dataset.txt.gz", "/home/someusername/workspace_local/orkut.txt") < 0) {
        printf("Uncompressing failed\n");
        return -1;
    }
    printf("Success\n");
    return 0;
}
