add_library(layout  random_layout.c)
#reorganize_records.c )
target_link_libraries(layout  PRIVATE query access data-struct)
