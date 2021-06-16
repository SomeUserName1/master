#include "query/louvain.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/operators.h"
#include "data-struct/dict_ul.h"
#include "query/snap_importer.h"

int
main(void)
{
    printf("Start querying\n");
    in_memory_file_t* db  = create_in_memory_file();
    dict_ul_ul_t*     map = import_from_txt(
          db, "/home/someusername/workspace_local/email_eu.txt");
    dict_ul_ul_destroy(map);

    printf("Starting the louvain method\n");
    unsigned long* partition = louvain(db);
    printf("Louvain method finished\n");

    FILE* in_c_file =
          fopen("/home/someusername/workspace_local/louvain_c_out", "r");
    FILE* in_cpp_file =
          fopen("/home/someusername/workspace_local/louvain_cpp_out", "r");

    if (!in_c_file || !in_cpp_file) {
        printf("louvain_test: Error opening the file!\n");
        exit(-1);
    }

    unsigned long* c_part = calloc(db->node_id_counter, sizeof(unsigned long));
    unsigned long* cpp_part =
          calloc(db->node_id_counter, sizeof(unsigned long));

    unsigned long node_id;
    unsigned long part_id;
    while (fscanf(in_c_file, "%lu %lu", &node_id, &part_id) == 2) {
        c_part[node_id] = part_id;
    }
    while (fscanf(in_cpp_file, "%lu %lu", &node_id, &part_id) == 2) {
        cpp_part[node_id] = part_id;
    }
    fclose(in_c_file);
    fclose(in_cpp_file);

    dict_ul_ul_t* c_mapping     = create_dict_ul_ul();
    dict_ul_ul_t* cpp_mapping   = create_dict_ul_ul();
    dict_ul_ul_t* c_cpp_mapping = create_dict_ul_ul();

    unsigned long c_error_count     = 0;
    unsigned long cpp_error_count   = 0;
    unsigned long c_cpp_error_count = 0;
    printf("Node, this, C, C++\n");

    for (size_t i = 0; i < db->node_id_counter; i++) {
        printf("%lu, %lu, %lu, %lu\n", i, partition[i], c_part[i], cpp_part[i]);

        if (!dict_ul_ul_contains(c_mapping, c_part[i])) {
            dict_ul_ul_insert(c_mapping, c_part[i], partition[i]);
        } else if (!(dict_ul_ul_get_direct(c_mapping, c_part[i])
                     == partition[i])) {
            c_error_count++;
        }

        if (!dict_ul_ul_contains(cpp_mapping, cpp_part[i])) {
            dict_ul_ul_insert(cpp_mapping, cpp_part[i], partition[i]);
        } else if (!(dict_ul_ul_get_direct(cpp_mapping, cpp_part[i])
                     == partition[i])) {
            cpp_error_count++;
        }

        if (!dict_ul_ul_contains(c_cpp_mapping, cpp_part[i])) {
            dict_ul_ul_insert(c_cpp_mapping, cpp_part[i], c_part[i]);
        } else if (!(dict_ul_ul_get_direct(c_cpp_mapping, cpp_part[i])
                     == c_part[i])) {
            c_cpp_error_count++;
        }
    }

    printf("Error between the paper author's implementations: %lu\n",
           c_cpp_error_count);
    printf("Error between this & the paper author's C implementation: %lu\n",
           c_error_count);
    printf("Error between this & the paper author's C++ implementation: %lu\n",
           cpp_error_count);

    dict_ul_ul_destroy(c_mapping);
    dict_ul_ul_destroy(cpp_mapping);
    dict_ul_ul_destroy(c_cpp_mapping);
    free(partition);
    free(c_part);
    free(cpp_part);
    in_memory_file_destroy(db);
}
