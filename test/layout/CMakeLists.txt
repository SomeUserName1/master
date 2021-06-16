find_package(ZLIB)
add_executable(reorganize_records-test  reorganize_records_test.c)
target_link_libraries(reorganize_records-test -lz -lcurl layout access query)


add_test("Remap IDs Test" reorganize_records-test)
