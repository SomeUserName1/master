find_package(ZLIB)
add_executable(louvain-test  lounvain_test.c)
target_link_libraries(louvain-test louvain import)

add_test("Louvain Test" louvain-test)
