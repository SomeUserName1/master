#include "../src/import/snap_importer.h" 

#include <stdio.h>
#include <assert.h>

#include "../src/access/in_memory_file.h"
#include "../src/data-struct/dict_ul.h"

int main(void) {
    printf("At least the entry point is right\n");
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

    assert(dict_ul_node_size(db->cache_nodes) == EMAIL_EU_CORE_NO_NODES);
    assert(dict_ul_rel_size(db->cache_rels) == EMAIL_EU_CORE_NO_RELS);

    printf("Success\n");
    return 0;
}
