add_executable(operators-test  operators_test.c)
target_link_libraries(operators-test  query)

add_executable(degree-test  degree_test.c)
target_link_libraries(degree-test query)

add_executable(bfs-test  bfs_test.c)
target_link_libraries(bfs-test query)

add_executable(dfs-test  dfs_test.c)
target_link_libraries(dfs-test query)

add_executable(dijkstra-test  dijkstra_test.c)
target_link_libraries(dijkstra-test query)

add_executable(a-star-test  a-star_test.c)
target_link_libraries(a-star-test query)

add_executable(alt-test  alt_test.c)
target_link_libraries(alt-test query)

add_executable(random-walk-test  random_walk_test.c)
target_link_libraries(random-walk-test query)

add_executable(louvain-test  louvain_test.c)
target_link_libraries(louvain-test query)

add_executable(snap-importer-test   snap_importer_test.c)
target_link_libraries(snap-importer-test query)

add_test("Operators Test" operators-test)
add_test("Import Test" snap-importer-test)
add_test("Degree Test" degree-test)
add_test("BFS Test" bfs-test)
add_test("DFS Test" dfs-test)
add_test("Dijkstra's Test" dijkstra-test)
add_test("A* Test" a-star-test)
add_test("ALT Test" alt-test)
add_test("Random Walk Test" random-walk-test)
add_test("Louvain Test" louvain-test)
