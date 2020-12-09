# Add subdirectories
add_subdirectory(query/)
add_subdirectory(storage/)

# Add executable
add_executable(gndb main.c)

# Link executable
target_link_libraries(gndb io cache record_formats storage)
