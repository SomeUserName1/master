find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

# louvain.c
add_library(query  degree.c result_types.c bfs.c dfs.c
    random_walk.c dijkstra.c a-star.c alt.c  snap_importer.c)

target_link_libraries(query
    PUBLIC  access
    PUBLIC   -lz -lcurl)
