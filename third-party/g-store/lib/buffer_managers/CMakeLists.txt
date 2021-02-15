add_library(buffer_managers bufmgr1.cpp bufmgr2.cpp bufmgr3.cpp lru.cpp)
target_include_directories(buffer_managers PRIVATE ..)
list(APPEND LIBS buffer_managers)
