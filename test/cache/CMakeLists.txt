add_executable(page-test   page_test.c)
target_include_directories(page-test PRIVATE ../../src/cache)
target_link_libraries(page-test cache)

add_executable(page-cache-test   page_cache_test.c)
target_include_directories(page-cache-test PRIVATE ../../src/cache)
target_link_libraries(page-cache-test data-struct cache)


add_test("Page Test" page-test)
add_test("Page Cache Test" page-cache-test)

