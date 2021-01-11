add_library(access  in_memory_file.c)

target_link_libraries(access  PUBLIC record data-struct)
