# Add subdirectories
add_subdirectory(data-struct/)
add_subdirectory(io)
add_subdirectory(cache)
add_subdirectory(access/)
add_subdirectory(query/)
add_subdirectory(layout/)


# Add executable
add_executable(gndb main.c)

# Link executable
target_link_libraries(gndb -lcurl -lz layout query access data-struct)
