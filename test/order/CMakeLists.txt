add_executable(reorder_records-test  reorganize_records_test.c)
target_link_libraries(reorder_records-test order access query)

add_executable(random-order-test random_layout_test.c)
target_link_libraries(random-order-test order access query)

add_test("Reorder Records Test" reorder_records-test)
add_test("Random Order test" random-order-test)
