add_executable(operators-test   operators_test.c)
target_link_libraries(operators-test   -lz -lcurl query access)

add_executable(ids_to_io-test  ids_to_io_test.c)
target_link_libraries(ids_to_io-test access)

add_executable(node-test test_node.c)
target_link_libraries(node-test access)

add_executable(rel-test test_relationship.c)
target_link_libraries(rel-test access)

add_test("Node Record Test" node-test)
add_test("Relationship Record Test" rel-test)
add_test("IDs to IOS Test" ids_to_io-test)
add_test("Operators" operators-test)
