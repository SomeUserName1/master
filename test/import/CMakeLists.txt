find_package(ZLIB)
add_executable(snap-importer-test   snap_importer_test.c)
target_link_libraries(snap-importer-test   -lz -lcurl import)

add_test("Import Test" snap-importer-test)
