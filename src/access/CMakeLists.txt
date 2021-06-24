add_library(access operators.c)
add_library(record node.c relationship.c)

target_link_libraries(access  PRIVATE data-struct record)

