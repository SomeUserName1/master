add_library(analyze  ids_to_io.c reorganize_records.c trivial.c)
target_link_libraries(analyze  PRIVATE record)
list(APPEND LIBS analyze)

add_library(icbl icbl.c)
target_link_libraries(icbl PRIVATE query access)
list(APPEND LIBS icbl)

add_library(g-store-layout g-store_layout.c)
target_link_libraries(g-store-layout PRIVATE access)
list(APPEND LIBS g-store-layout)

