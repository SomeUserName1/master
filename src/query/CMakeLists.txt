add_library(query  degree.c result_types.c bfs.c dfs.c random_walk.c dijkstra.c a-star.c alt.c louvain.c)

target_link_libraries(query
    PUBLIC  access
    PUBLIC  data-struct)
