add_library(io disk_file.c physical_database.c)
target_link_libraries(io strace)
