find_package(ZLIB)
add_executable(g-store-test  g-store_layout_test.c)
target_link_libraries(g-store-test -lz -lcurl g-store-layout import)

add_executable(icbl-test  icbl_test.c)
target_link_libraries(icbl-test -lz -lcurl icbl import)

add_executable(ids_to_io-test  ids_to_io_test.c)
target_link_libraries(ids_to_io-test analyze)

add_executable(reorganize_records-test  reorganize_records_test.c)
target_link_libraries(reorganize_records-test -lz -lcurl analyze query import)


add_test("G-Store Test" g-store-test)
add_test("ICBL Test" icbl-test)
add_test("IDs to IOS Test" ids_to_io-test)
add_test("Remap IDs Test" reorganize_records-test)
