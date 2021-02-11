add_subdirectory(buffer_managers/)
add_subdirectory(query/)
add_subdirectory(multilevel/)

add_library(g-store-lib block.cpp calc_header.cpp cmd_input.cpp entry_points.cpp evaluate.cpp fifo.cpp find_blocksize.cpp memory_mgr.cpp move_to_disk.cpp page_queue_elevator.cpp page_queue_rewind.cpp parameters.cpp pred_tree.cpp print_graph.cpp read_input.cpp read_query.cpp read_schema.cpp read_to_blocks.cpp read_to_psql_ora.cpp structs.cpp util.cpp util_s.cpp)
list(APPEND LIBS g-store)
