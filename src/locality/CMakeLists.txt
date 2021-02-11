add_library(analyze  ids_to_io.c)

target_link_libraries(analyze  PRIVATE record)
