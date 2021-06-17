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
    free(louvain(db));
    printf("Louvain method finished\n");

    in_memory_file_destroy(db);
}
