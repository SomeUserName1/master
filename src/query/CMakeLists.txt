find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

add_library(query  heap_file_operators.c in_memory_operators.c degree.c result_types.c bfs.c dfs.c
    random_walk.c dijkstra.c a-star.c alt.c louvain.c snap_importer.c)

target_link_libraries(query
    PUBLIC  access
    PUBLIC   -lz -lcurl)
