add_library(cache page_cache.c page.c)
target_include_directories(cache PRIVATE ../io)
target_link_libraries(cache PRIVATE io data-struct)
