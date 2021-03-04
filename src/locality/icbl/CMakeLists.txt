add_library(icbl icbl.c)
target_link_libraries(icbl PRIVATE query access)
list(APPEND LIBS icbl)
