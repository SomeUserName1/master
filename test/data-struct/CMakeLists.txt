add_executable(data-struct-test dict_ul_test.c)

target_link_libraries(data-struct-test data-struct)

add_test("Testing dictionary structs" data-struct-test)
