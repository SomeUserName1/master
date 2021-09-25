add_executable(reorganize_records-test  reorganize_records_test.c)
target_link_libraries(reorganize_records-test layout access query)

add_executable(random-layout-test random_layout_test.c)

add_test("Reorder Records Test" reorganize_records-test)
add_test("Random Layout test" random-layout-test)
