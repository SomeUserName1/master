add_executable(dict-test dict_ul_test.c)
target_link_libraries(dict-test -lz -lcurl data-struct access query)

add_executable(list-test list_test.c)
target_link_libraries(list-test data-struct)

add_executable(queue-test queue_ul_test.c)
target_link_libraries(queue-test data-struct)

add_executable(set-test set_ul_test.c)
target_link_libraries(set-test data-struct)

add_executable(fibonacci_heap-test fibonacci_heap_test.c)
target_link_libraries(fibonacci_heap-test data-struct)

add_test("Dictionary Test" dict-test)
add_test("List Test" list-test)
add_test("Queue Test" queue-test)
add_test("Set Test" set-test)
add_test("Fibonacci Heap" fibonacci_heap-test)
