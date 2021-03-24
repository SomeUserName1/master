find_package(ZLIB)
add_executable(g-store-test  g-store_layout_test.c)
target_link_libraries(g-store-test -lz -lcurl g-store import)

add_executable(icbl-test  icbl_test.c)
target_link_libraries(icbl-test -lz -lcurl icbl import)

add_executable(ids_to_io-test  ids_to_io_test.c)
target_link_libraries(ids_to_io-test locality)

add_executable(remap_ids-test  remap_ids_test.c)
target_link_libraries(remap_ids-test locality)


add_test("G-Store Test" g-store-test)
add_test("ICBL Test" ICBL-test)
add_test("IDs to IOS Test" ids_to_ios-test)
add_test("Remap IDs Test" remap_ids-test)
