add_executable(in-memory-file-test   in_memory_file_test.c)
target_link_libraries(in-memory-file-test   -lz -lcurl import)

add_test("Access layer Test" in-memory-file-test)
