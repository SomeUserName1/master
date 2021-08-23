add_library(access heap_file.c in_memory_graph.c node.c relationship.c header_page.c)
target_include_directories(access PUBLIC ../cache ../io)
target_link_libraries(access PUBLIC cache data-struct)
