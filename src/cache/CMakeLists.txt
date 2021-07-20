add_library(cache page_cache.c page.c)
target_include_directories(cache PUBLIC ../io)
target_link_libraries(cache PUBLIC io)
