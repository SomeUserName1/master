add_library(data-struct  cbs.c
                         htable.c
                         list.c
                         queue.c
                         dict_ul.c
                         list_node.c
                         list_rel.c
                         list_ul.c
                         list_l.c
                         queue_ul.c
                         set.c
                         set_ul.c
                         fibonacci_heap.c)
target_link_libraries(data-struct PUBLIC record)
