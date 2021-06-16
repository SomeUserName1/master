#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "access/operators.h"
#include "data-struct/dict_ul.h"
#include "layout/random_layout.h"
#include "layout/reorganize_records.h"
#include "query/a-star.h"
#include "query/alt.h"
#include "query/bfs.h"
#include "query/dfs.h"
#include "query/dijkstra.h"
#include "query/louvain.h"
#include "query/result_types.h"
#include "query/snap_importer.h"

#define NUM_LANDMARKS  (3)
#define PERMISSION_NUM (0777)

void
rek_mkdir(char* path)
{
    char* sep = strrchr(path, '/');
    if (sep != NULL) {
        *sep = 0;
        rek_mkdir(path);
        *sep = '/';
        if (mkdir(path, PERMISSION_NUM) && errno != EEXIST) {
            printf("error while trying to create '%s'\n%m\n", path);
        }
    }
}

int
main(void)
{
    // Path schema for results: base_path / dataset / layout_str /
    // (sorted_il)? query_str _ (records|blocks)
    const char* trash_file    = "/home/someusername/Downloads/ignore.txt";
    const char* base_path     = "/home/someusername/workspace_local/";
    const char* download_temp = "download.txt.gz";
    const char* layout_str[]  = { "natural", "random", "louvain" };
    const char* dataset_str[] = {
        "c_elegans", "email_research_eu", "dblp", "amazon", "youtube"
    };
    const char* query_str[] = { "bfs", "dfs", "dijkstra", "a-star", "alt" };

    typedef unsigned long* (*layout)(in_memory_file_t * db);

    layout layout_method[] = { identity_partition, random_partition, louvain };

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

    //    for (dataset_t dataset = 0; dataset <= YOUTUBE; dataset++) {
    //        printf("Downloading & uncompressing dataset %s ...\n",
    //               dataset_str[dataset]);
    //        download_dataset(dataset, temp_path);
    //        uncompress_dataset(temp_path, dataset_path[dataset]);
    //        printf("Done!\n");
    //    }
    free(temp_path);

    // In order to avoid effects from different initializations, we reimport
    // every dataset for all layouts. That is, all layout algorithm start
    // with the "natural" order imposed by the dataset.
    in_memory_file_t* db;
    unsigned long     source_node;
    unsigned long     target_node;
    unsigned long*    partition;
    char*             result_base_path;
    char*             result_specific_path;
    char*             result_specific_block_path;
    char*             result_specific_page_path;
    char*             mem_alloc_h;
    double*           heuristic;
    double**          landmark_dists = malloc(NUM_LANDMARKS * sizeof(double*));
    sssp_result*      dijk_result;
    unsigned long     n_layout_methods = 5;

    srand(time(NULL));

    for (dataset_t dataset = 3; dataset <= YOUTUBE; ++dataset) {
        printf("Using dataset %s =========================\n",
               dataset_str[dataset]);
        source_node = rand() % get_no_nodes(dataset);
        target_node = rand() % get_no_nodes(dataset);

        if (dataset >= DBLP) {
            n_layout_methods = 4;
        }

        for (size_t i = 0; i < n_layout_methods; ++i) {
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
            rek_mkdir(result_base_path);
            printf("Base path: %s\n", result_base_path);

            // Preprocess the landmarks for alt to avoid doing it twice.
            printf("Preprocessing heuristic and landmarks for A* and ALT\n");
            alt_preprocess(db, BOTH, NUM_LANDMARKS, landmark_dists, trash_file);
            dijk_result = dijkstra(db, target_node, BOTH, trash_file);
            heuristic   = dijk_result->distances;
            free(dijk_result->pred_edges);
            free(dijk_result);

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

                    printf("Base path: %s\n", result_base_path);
                    printf("Rerun queries\n");
                } else {
                    printf("Run queries ===========\n");
                }

                // Construct ouput path & run BFS
                printf("Running %s with source node %lu\n",
                       query_str[0],
                       source_node);

                result_specific_path =
                      calloc(strlen(result_base_path) + strlen(query_str[0])
                                   + strlen("_ids.txt") + 1,
                             sizeof(char));
                result_specific_block_path =
                      calloc(strlen(result_base_path) + strlen(query_str[0])
                                   + strlen("_blocks.txt") + 1,
                             sizeof(char));
                result_specific_page_path =
                      calloc(strlen(result_base_path) + strlen(query_str[0])
                                   + strlen("_pages.txt") + 1,
                             sizeof(char));

                if (!result_specific_path) {
                    printf("main - memory allocation failed\n");
                    exit(-1);
                }

                strncat(result_specific_path,
                        result_base_path,
                        strlen(result_base_path));
                strncat(
                      result_specific_path, query_str[0], strlen(query_str[0]));

                strncat(result_specific_block_path,
                        result_specific_path,
                        strlen(result_specific_path));
                strncat(result_specific_page_path,
                        result_specific_path,
                        strlen(result_specific_path));

                strncat(result_specific_path, "_ids.txt", strlen("_ids.txt"));
                strncat(result_specific_block_path,
                        "_blocks.txt",
                        strlen("_blocks.txt"));
                strncat(result_specific_page_path,
                        "_pages.txt",
                        strlen("_pages.txt"));

                fclose(fopen(result_specific_path, "w"));

                traversal_result_destroy(
                      bfs(db, source_node, BOTH, result_specific_path));
                ids_to_blocks(
                      result_specific_path, result_specific_block_path, ALL);
                blocks_to_pages(result_specific_block_path,
                                result_specific_page_path,
                                ALL);
                printf("Done\n");

                // Construct ouput path & run DFS
                printf("Running %s with source node %lu\n",
                       query_str[1],
                       source_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[1])
                                    + strlen("_ids.txt") + 1);

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                free(result_specific_block_path);
                free(result_specific_page_path);
                result_specific_block_path =
                      calloc(strlen(result_base_path) + strlen(query_str[1])
                                   + strlen("_blocks.txt") + 1,
                             sizeof(char));

                result_specific_page_path =
                      calloc(strlen(result_base_path) + strlen(query_str[1])
                                   + strlen("_pages.txt") + 1,
                             sizeof(char));

                result_specific_path[strlen(result_base_path)] = '\0';

                strncat(
                      result_specific_path, query_str[1], strlen(query_str[1]));

                strncat(result_specific_block_path,
                        result_specific_path,
                        strlen(result_specific_path));
                strncat(result_specific_page_path,
                        result_specific_path,
                        strlen(result_specific_path));

                strncat(result_specific_path, "_ids.txt", strlen("_ids.txt"));
                strncat(result_specific_block_path,
                        "_blocks.txt",
                        strlen("_blocks.txt"));
                strncat(result_specific_page_path,
                        "_pages.txt",
                        strlen("_pages.txt"));

                fclose(fopen(result_specific_path, "w"));
                fclose(fopen(result_specific_block_path, "w"));
                fclose(fopen(result_specific_page_path, "w"));

                traversal_result_destroy(
                      dfs(db, source_node, BOTH, result_specific_path));
                ids_to_blocks(
                      result_specific_path, result_specific_block_path, ALL);
                blocks_to_pages(result_specific_block_path,
                                result_specific_page_path,
                                ALL);

                printf("Done\n");

                // Construct ouput path & run dijkstra
                printf("Running %s with source node %lu\n",
                       query_str[2],
                       source_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[2])
                                    + strlen("_ids.txt") + 1);

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                free(result_specific_block_path);
                free(result_specific_page_path);
                result_specific_block_path =
                      calloc(strlen(result_base_path) + strlen(query_str[2])
                                   + strlen("_blocks.txt") + 1,
                             sizeof(char));

                result_specific_page_path =
                      calloc(strlen(result_base_path) + strlen(query_str[2])
                                   + strlen("_pages.txt") + 1,
                             sizeof(char));

                result_specific_path[strlen(result_base_path)] = '\0';

                strncat(
                      result_specific_path, query_str[2], strlen(query_str[2]));

                strncat(result_specific_block_path,
                        result_specific_path,
                        strlen(result_specific_path));
                strncat(result_specific_page_path,
                        result_specific_path,
                        strlen(result_specific_path));

                strncat(result_specific_path, "_ids.txt", strlen("_ids.txt"));
                strncat(result_specific_block_path,
                        "_blocks.txt",
                        strlen("_blocks.txt"));
                strncat(result_specific_page_path,
                        "_pages.txt",
                        strlen("_pages.txt"));

                fclose(fopen(result_specific_path, "w"));
                fclose(fopen(result_specific_block_path, "w"));
                fclose(fopen(result_specific_page_path, "w"));

                sssp_result_destroy(
                      dijkstra(db, source_node, BOTH, result_specific_path));
                ids_to_blocks(
                      result_specific_path, result_specific_block_path, ALL);
                blocks_to_pages(result_specific_block_path,
                                result_specific_page_path,
                                ALL);
                printf("Done\n");

                // Construct ouput path & run A*
                printf("Running %s with source node %lu, and target node "
                       "%lu\n",
                       query_str[3],
                       source_node,
                       target_node);

                mem_alloc_h =
                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[3])
                                    + strlen("_ids.txt") + 1);

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                free(result_specific_block_path);
                free(result_specific_page_path);
                result_specific_block_path =
                      calloc(strlen(result_base_path) + strlen(query_str[3])
                                   + strlen("_blocks.txt") + 1,
                             sizeof(char));

                result_specific_page_path =
                      calloc(strlen(result_base_path) + strlen(query_str[3])
                                   + strlen("_pages.txt") + 1,
                             sizeof(char));

                result_specific_path[strlen(result_base_path)] = '\0';

                strncat(
                      result_specific_path, query_str[3], strlen(query_str[3]));

                strncat(result_specific_block_path,
                        result_specific_path,
                        strlen(result_specific_path));
                strncat(result_specific_page_path,
                        result_specific_path,
                        strlen(result_specific_path));

                strncat(result_specific_path, "_ids.txt", strlen("_ids.txt"));
                strncat(result_specific_block_path,
                        "_blocks.txt",
                        strlen("_blocks.txt"));
                strncat(result_specific_page_path,
                        "_pages.txt",
                        strlen("_pages.txt"));

                fclose(fopen(result_specific_path, "w"));
                fclose(fopen(result_specific_block_path, "w"));
                fclose(fopen(result_specific_page_path, "w"));

                path_destroy(a_star(db,
                                    heuristic,
                                    source_node,
                                    target_node,
                                    BOTH,
                                    result_specific_path));
                ids_to_blocks(
                      result_specific_path, result_specific_block_path, ALL);
                blocks_to_pages(result_specific_block_path,
                                result_specific_page_path,
                                ALL);
                printf("Done\n");

                // Construct ouput path & run alt
                printf("Running %s with source node %lu and target node "
                       "%lu\n",
                       query_str[4],
                       source_node,
                       target_node);

                mem_alloc_h =

                      realloc(result_specific_path,
                              strlen(result_base_path) + strlen(query_str[4])
                                    + strlen("_ids.txt") + 1);

                if (!mem_alloc_h) {
                    free(result_specific_path);
                    printf("main: Failed to allocate memory!\n");
                    exit(-1);
                } else {
                    result_specific_path = mem_alloc_h;
                }

                free(result_specific_block_path);
                free(result_specific_page_path);
                result_specific_block_path =
                      calloc(strlen(result_base_path) + strlen(query_str[4])
                                   + strlen("_blocks.txt") + 1,
                             sizeof(char));

                result_specific_page_path =
                      calloc(strlen(result_base_path) + strlen(query_str[4])
                                   + strlen("_pages.txt") + 1,
                             sizeof(char));

                result_specific_path[strlen(result_base_path)] = '\0';

                strncat(
                      result_specific_path, query_str[4], strlen(query_str[4]));

                strncat(result_specific_block_path,
                        result_specific_path,
                        strlen(result_specific_path));
                strncat(result_specific_page_path,
                        result_specific_path,
                        strlen(result_specific_path));

                strncat(result_specific_path, "_ids.txt", strlen("_ids.txt"));
                strncat(result_specific_block_path,
                        "_blocks.txt",
                        strlen("_blocks.txt"));
                strncat(result_specific_page_path,
                        "_pages.txt",
                        strlen("_pages.txt"));

                fclose(fopen(result_specific_path, "w"));
                fclose(fopen(result_specific_block_path, "w"));
                fclose(fopen(result_specific_page_path, "w"));

                path_destroy(alt(db,
                                 landmark_dists,
                                 3,
                                 source_node,
                                 target_node,
                                 BOTH,
                                 result_specific_path));
                ids_to_blocks(
                      result_specific_path, result_specific_block_path, ALL);
                blocks_to_pages(result_specific_block_path,
                                result_specific_page_path,
                                ALL);
                printf("Done\n");

                free(result_specific_path);
                free(result_specific_block_path);
                free(result_specific_page_path);
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
