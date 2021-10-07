add_library(order  random_order.c reorder_records.c )
target_link_libraries(order  PRIVATE query access data-struct)
