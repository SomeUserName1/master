# Add subdirectories
add_subdirectory(gen_data_types/)
add_subdirectory(query/)
add_subdirectory(storage/)

# Add executable
add_executable(gndb main.c)

# Link executable
target_link_libraries(gndb records cache io query)
