add_library(data-struct array_list.c cbs.c fibonacci_heap.c htable.c linked_list.c set.c)
target_include_directories(data-struct PRIVATE ../cache)
target_link_libraries(data-struct PRIVATE record)
