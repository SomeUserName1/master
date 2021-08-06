add_executable(heap-file-test   test_heap_file.c)
target_link_libraries(heap-file-test  access)

add_executable(in-memory-graph-test   test_in_memory_graph.c)
target_link_libraries(in-memory-graph-test  access)

add_executable(node-test test_node.c)
target_link_libraries(node-test access)

add_executable(rel-test test_relationship.c)
target_link_libraries(rel-test access)

add_test("Node Record Test" node-test)
add_test("Relationship Record Test" rel-test)
add_test("Operators" heap-file-test)
