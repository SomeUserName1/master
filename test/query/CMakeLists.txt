find_package(ZLIB)
add_executable(louvain-test  louvain_test.c)
target_link_libraries(louvain-test -lz -lcurl query import)

add_test("Louvain Test" louvain-test)
