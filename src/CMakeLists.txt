# Add subdirectories
add_subdirectory(record/)
add_subdirectory(data-struct)
# add_subdirectory(io)
# add_subdirectory(cache)
add_subdirectory(access/)
add_subdirectory(query/)
add_subdirectory(import/)
add_subdirectory(locality/)


# Add executable
add_executable(gndb main.c)

# Link executable
target_link_libraries(gndb -lcurl -lz analyze g-store-layout icbl import query access data-struct)
