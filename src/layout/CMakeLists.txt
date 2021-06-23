add_library(layout  reorganize_nodes.c reorganize_relationships.c random_layout.c)
target_link_libraries(layout  PRIVATE access)
