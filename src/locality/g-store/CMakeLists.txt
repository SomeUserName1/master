add_library(g-store-layout g-store_layout.c)
target_link_libraries(g-store-layout PRIVATE access)
list(APPEND LIBS g-store-layout)
