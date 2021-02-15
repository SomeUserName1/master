add_executable(g-store main.cpp)
target_include_directories(g-store PRIVATE ../lib)
target_link_libraries(g-store PRIVATE ${LIBS})
