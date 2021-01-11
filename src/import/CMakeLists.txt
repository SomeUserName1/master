find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

add_library(import      snap_importer.c)

target_link_libraries(import 
    PUBLIC access)
