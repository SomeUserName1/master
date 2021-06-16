add_library(access operators.c node.c relationship.c)

target_link_libraries(access  PRIVATE data-struct)

