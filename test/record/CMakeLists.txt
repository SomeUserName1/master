add_executable(node-test test_node.c)
target_link_libraries(node-test record)

add_executable(rel-test test_relationship.c)
target_link_libraries(rel-test record)

add_test("Node Record Test" node-test)
add_test("Relationship Record Test" rel-test)
