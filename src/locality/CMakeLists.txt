add_library(analyze  ids_to_io.c reorganize_records.c)

add_subdirectory(g-store/)
add_subdirectory(icbl/)

target_link_libraries(analyze  PRIVATE record)
