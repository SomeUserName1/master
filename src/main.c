#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "access/in_memory_file.h"
#include "constants.h"
#include "data-struct/dict_ul.h"
#include "import/snap_importer.h"
#include "locality/g-store_layout.h"
#include "locality/icbl.h"
#include "locality/ids_to_io.h"
#include "locality/reorganize_records.h"
#include "locality/trivial.h"
#include "query/a-star.h"
#include "query/alt.h"
#include "query/bfs.h"
#include "query/dfs.h"
#include "query/dijkstra.h"
#include "query/louvain.h"
#include "query/result_types.h"

#define NUM_LAYOUT_METHOS (5)

int
main(void)
{
    // Path schema for results: base_path / dataset / layout_str / (sorted_il)?
    // query_str _ (records|blocks)
    const char* trash_file    = "/home/someusername/Downloads/ignore.txt";
    const char* base_path     = "/home/someusername/workspace_local/";
    const char* download_temp = "download.txt.gz";
    const char* layout_str[]  = {
        "natural", "random", "louvain", "g-store", "icbl"
    };
    const char* dataset_str[] = {
        "c_elegans", "email_research_eu", "dblp", "amazon", "youtube"
    };
    const char* query_str[] = { "bfs", "dfs", "dijkstra", "a-star", "alt" };

    typedef unsigned long* (*layout)(in_memory_file_t * db);

    layout layout_method[] = {
        identity_partition, random_partition, louvain, g_store_layout, icbl
    };

    // Build download temp string.
    char* temp_path =
          calloc(strlen(base_path) + strlen(download_temp) + 1, sizeof(char));
    strncat(temp_path, base_path, strlen(base_path));
    strncat(temp_path, download_temp, strlen(download_temp));

    // Build the paths for the dataset txt files.
    // Youtube marks the number in the enum. The enum is sorted by size and
    // starts at 0. I.e. we use idx(Youtube) + 1 datasets.
    char* dataset_path[YOUTUBE + 1];

    for (size_t i = 0; i <= YOUTUBE; ++i) {
        dataset_path[i] = calloc(
              strlen(base_path) + strlen(dataset_str[i]) + 4 + 1, sizeof(char));
        strncat(dataset_path[i], base_path, strlen(base_path));
        strncat(dataset_path[i], dataset_str[i], strlen(dataset_str[i]));
        strncat(dataset_path[i], ".txt", strlen(".txt"));
    }

    for (dataset_t dataset = 0; dataset <= YOUTUBE; dataset++) {
        printf("Downloading & uncompressing dataset %s ...\n",
               dataset_str[dataset]);
        download_dataset(dataset, temp_path);
        uncompress_dataset(temp_path, dataset_path[dataset]);
        printf("Done!\n");
    }
    free(temp_path);

    // In order to avoid effects from different initializations, we reimport
    // every dataset for all layouts. That is, all layout algorithm start with
    // the "natural" order imposed by the dataset.
    in_memory_file_t* db;
    unsigned long     source_node;
    unsigned long     target_node;
    unsigned long*    partition;
    char*             result_base_path;
    char*             result_specific_path;
    char*             mem_alloc_h;
    double*           heuristic;
    double**          landmark_dists = malloc(3 * sizeof(double*));

    srand(time(NULL));

    for (dataset_t dataset = 0; dataset <= YOUTUBE; ++dataset) {
        printf("Using dataset %s =========================\n",
               dataset_str[dataset]);
        source_node = rand() % get_no_nodes(dataset);
        target_node = rand() % get_no_nodes(dataset);

        heuristic = calloc(get_no_nodes(dataset), sizeof(double));

        for (size_t i = 0; i < NUM_LAYOUT_METHOS; ++i) {
            printf("Using layout method %s ===============\n", layout_str[i]);

            db = create_in_memory_file();

            printf("Importing...\n");
            dict_ul_ul_destroy(import_from_txt(db, dataset_path[dataset]));
            printf("Done!\n");

            printf("Applying layout method...\n");
            partition = layout_method[i](db);
            reorganize_records(db, partition);
            printf("Done\n");

            // base_path / dataset / layout_method / (sorted_il)? query
            result_base_path =
                  calloc(strlen(base_path) + strlen(dataset_str[dataset]) + 1
                               + strlen(layout_str[i]) + 1 + 1,
                         sizeof(char));

            strncat(result_base_path, base_path, strlen(base_path));
            strncat(result_base_path,
                    dataset_str[dataset],
                    strlen(dataset_str[dataset]));
            strncat(result_base_path, "/", 1);
            strncat(result_base_path, layout_str[i], strlen(layout_str[i]));
            strncat(result_base_path, "/", 1);

            // Preprocess the landmarks for alt to avoid doing it twice.
            alt_preprocess(db, BOTH, 3, landmark_dists, trash_file);

            for (size_t k = 0; k < 2; ++k) {
                if (k) {
                    printf("Reordering incidence list pointers ===== \n");
                    sort_incidence_list(db);

                    mem_alloc_h = realloc(
                          result_base_path,
                          strlen(base_path) + strlen(dataset_str[dataset]) + 1
                                + strlen(layout_str[i]) + 1 + 4 + 1);

                    if (!mem_alloc_h) {
                        free(result_base_path);
                        printf("main: Failed to allocate memory!\n");
                        exit(-1);
                    } else {
                        result_base_path = mem_alloc_h;
                    }

                    strncat(result_base_path, "sil_", strlen("sil_"));
                    printf("Rerun queries\n");
                } else {
                    printf("Run queries ===========\n");
                }

                // Construct ouput path & run BFS
                printf("Running %s with source node %lu\n",
                       query_str[0],
                       source_node);

                result_specific_path =
                      calloc(strlen(result_base_path) + strlen(query_str[0]),
                             sizeof(char));

                strncat(
                      result_specific_path, query_str[0], strlen(query_str[0]));

                strncat(result_specific_path, ".txt", 4);

                traversal_result_destroy(
                      bfs(db, source_node, BOTH, result_specific_path));
                printf("Done\n");

                // Construct ouput path & run DFS
                printf("Running %s with source node %lu\n",
                       query_str[1],
                       source_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[1]));

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                result_specific_path[strlen(result_base_path) + 1] = '\0';

                strncat(
                      result_specific_path, query_str[1], strlen(query_str[1]));

                strncat(result_specific_path, ".txt", 4);

                traversal_result_destroy(
                      dfs(db, source_node, BOTH, result_specific_path));
                printf("Done\n");

                // Construct ouput path & run dijkstra
                printf("Running %s with source node %lu\n",
                       query_str[2],
                       source_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[2]));

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                result_specific_path[strlen(result_base_path) + 1] = '\0';

                strncat(
                      result_specific_path, query_str[2], strlen(query_str[2]));

                strncat(result_specific_path, ".txt", 4);

                sssp_result_destroy(
                      dijkstra(db, source_node, BOTH, result_specific_path));
                printf("Done\n");

                // Construct ouput path & run A*
                printf("Running %s with source node %lu, and target node %lu\n",
                       query_str[3],
                       source_node,
                       target_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[3]));

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                result_specific_path[strlen(result_base_path) + 1] = '\0';

                strncat(
                      result_specific_path, query_str[3], strlen(query_str[3]));

                strncat(result_specific_path, ".txt", 4);

                path_destroy(a_star(db,
                                    heuristic,
                                    source_node,
                                    target_node,
                                    BOTH,
                                    result_specific_path));
                printf("Done\n");

                // Construct ouput path & run dijkstra
                printf("Running %s with source node %lu and target node %lu\n",
                       query_str[4],
                       source_node,
                       target_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[4]));

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                result_specific_path[strlen(result_base_path) + 1] = '\0';

                strncat(
                      result_specific_path, query_str[4], strlen(query_str[4]));

                strncat(result_specific_path, ".txt", 4);

                path_destroy(alt(db,
                                 landmark_dists,
                                 3,
                                 source_node,
                                 target_node,
                                 BOTH,
                                 result_specific_path));
                printf("Done\n");

                free(result_specific_path);
            }

            for (size_t l = 0; l < 3; ++l) {
                printf("free %lu\n", l);
                free(landmark_dists[i]);
            }
            in_memory_file_destroy(db);
            free(result_base_path);
        }
        free(heuristic);
    }

    free(landmark_dists);

    for (size_t i = 0; i <= YOUTUBE; ++i) {
        free(dataset_path[i]);
    }

    return 0;
}
