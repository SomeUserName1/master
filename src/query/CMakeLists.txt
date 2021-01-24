add_library(query  bfs.c louvain.c)

target_link_libraries(query
    PUBLIC  access
    PUBLIC  data-struct)
