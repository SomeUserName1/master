add_library(layout  reorganize_records.c random_layout.c)
target_link_libraries(layout  PRIVATE access)
