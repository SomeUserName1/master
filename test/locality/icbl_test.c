#include "../../src/locality/icbl/icbl.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/constants.h"
#include "../../src/data-struct/dict_ul.h"
#include "../../src/import/snap_importer.h"

int
main(void)
{
    printf("Start importing\n");
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_t* map = import_from_txt(
          db, "/home/someusername/workspace_local/email_eu.txt");
    dict_ul_ul_destroy(map);

    printf("Start applying the ICBL multilevel partitioning algorithm.\n");
    unsigned long* partition = icbl(db);
    printf("Done.\n");

    FILE* out_f =
          fopen("/home/someusername/workspace_local/g-g_store_layout.txt", "w");

    if (!out_f) {
        printf("Couldn't open file");
        exit(-1);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        fprintf(out_f, "%lu %lu", i, partition[i]);
    }

    fclose(out_f);
    free(partition);
    in_memory_file_destroy(db);
    return 0;
}
