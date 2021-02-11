add_library(multilevel ml_coarsen.cpp ml_part.cpp ml_project.cpp ml_refine.cpp ml_reorder.cpp ml_turn_around.cpp ml_uncoarsen.cpp finalize.cpp)
list(APPEND LIBS multilevel)
