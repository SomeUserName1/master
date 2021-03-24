add_library(analyze  ids_to_io.c remap_ids.c)

add_subdirectory(g-store/)
add_subdirectory(icbl/)

target_link_libraries(analyze  PRIVATE record)
