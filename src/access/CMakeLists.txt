add_library(access heap_file.c in_memory_file.c node.c relationship.c)
target_include_directories(access PRIVATE ../cache)

target_link_libraries(access PRIVATE cache data-struct)
