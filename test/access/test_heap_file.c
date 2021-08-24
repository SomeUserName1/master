#include "access/heap_file.h"

void
test_heap_file_create(void)
{}

void
test_heap_file_destroy(void)
{}

void
test_create_node(void)
{}

void
test_create_relationship(void)
{}

void
test_read_node(void)
{}

void
test_read_relationship(void)
{}

void
test_update_node(void)
{}

void
test_update_relationship(void)
{}

void
test_delete_node(void)
{}

void
test_delete_relationship(void)
{}

void
test_move_node(void)
{}

void
test_move_relationship(void)
{}

void
test_swap_page(void)
{}

void
test_get_nodes(void)
{}

void
test_get_relationships(void)
{}

void
test_next_relationship_id(void)
{}

void
test_expand(void)
{}

void
test_contains_relationship_from_to(void)
{}

int
main(void)
{
    test_heap_file_create();
    test_heap_file_destroy();
    test_create_node();
    test_create_relationship();
    test_read_node();
    test_read_relationship();
    test_update_node();
    test_update_relationship();
    test_delete_node();
    test_delete_relationship();
    test_move_node();
    test_move_relationship();
    test_swap_page();
    test_get_nodes();
    test_get_relationships();
    test_next_relationship_id();
    test_expand();
    test_contains_relationship_from_to();

    return 0;
}
