#include "../../src/query/louvain.h"
#include "../../src/import/snap_importer.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

    unsigned long* partition = louvain(db);
    FILE* in_file = fopen("/home/someusername/workspace_local/louvain_c_out", "r");

    unsigned long* true_part = calloc(db->node_id_counter, sizeof(unsigned long));
    unsigned long node_id, part_id;
    while(fscanf(in_file, "%lu %lu", &node_id, &part_id) == 2) {
        true_part[node_id] = part_id;
    }

    dict_ul_ul_t* mapping = create_dict_ul_ul();

    for (size_t i = 0; i < db->node_id_counter; i++) {
        if (!dict_ul_ul_contains(mapping, true_part[i])) {
            dict_ul_ul_insert(mapping, true_part[i], partition[i]);
        } else {
            assert(dict_ul_ul_get_direct(mapping, true_part[i]) == partition[i]);
        }
    }
    dict_ul_ul_destroy(mapping);
    free(partition);
    free(true_part);
    in_memory_file_destroy(db);
    return 0;
}