add_library(access heap_file.c node.c relationship.c)

target_link_libraries(access PRIVATE cache data-struct)
