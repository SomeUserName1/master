add_executable(disk-file-test   disk_file_test.c)
target_include_directories(disk-file-test PRIVATE ../../src/io)
target_link_libraries(disk-file-test io)

add_executable(physical-database-test   phy_database_test.c)
target_include_directories(physical-database-test PRIVATE ../../src/io)
target_link_libraries(physical-database-test io)


add_test("Disk File Test" disk-file-test)
add_test("Physical Database Test" physical-database-test)

