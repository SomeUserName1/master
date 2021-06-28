add_executable(reorganize_nodes-test  reorganize_nodes_test.c)
target_link_libraries(reorganize_nodes-test layout access query)

add_executable(reorganize_rels-test  reorganize_rels_test.c)
target_link_libraries(reorganize_rels-test layout access query)

add_executable(random-layout-test random_layout_test.c)


add_test("Remap Node IDs Test" reorganize_nodes-test)
add_test("Remap Relationship IDs Test" reorganize_rels-test)
add_test("Random Layout test" random-layout-test)
