#include "data-struct/fibonacci_heap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void
test_create_fh(void)
{
    fib_heap_t* fh = create_fib_heap();

    assert(fh);
    assert(fh->num_nodes == 0);
    assert(!fh->min);

    fib_heap_destroy(fh);
}

void
test_create_fh_node(void)
{
    const double        key   = 42.42;
    const unsigned long value = 777;

    fib_node* node = create_fib_node(key, value);

    assert(node);
    assert(node->key == key);
    assert(node->value == value);
    assert(!node->child);
    assert(!node->mark);
    assert(!node->parent);
    assert(!node->right);
    assert(!node->left);
    assert(node->degree == 0);

    free(node);
}

void
test_fh_insert(void)
{
    const double        key   = 42.42;
    const unsigned long value = 777;

    fib_heap_t* fh   = create_fib_heap();
    fib_node*   node = create_fib_node(key, value);

    fib_heap_insert(fh, node);

    assert(fh->num_nodes == 1);
    assert(fh->min == node);
    assert(node->left == node);
    assert(node->right == node);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;

    fib_node* s_node = create_fib_node(s_key, s_value);

    fib_heap_insert(fh, s_node);

    assert(fh->min == s_node);
    assert(fh->num_nodes == 2);
    assert(node->right == s_node);
    assert(node->left == s_node);
    assert(s_node->left == node);
    assert(s_node->right == node);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;

    fib_node* m_node = create_fib_node(m_key, m_value);

    fib_heap_insert(fh, m_node);

    assert(fh->min == s_node);
    assert(fh->num_nodes == 3);
    assert(s_node->right == m_node);
    assert(m_node->right == node);
    assert(node->right == s_node);
    assert(s_node->left == node);
    assert(node->left == m_node);
    assert(m_node->left == s_node);

    fib_heap_destroy(fh);
}

void
test_fh_min(void)
{
    fib_heap_t* fh = create_fib_heap();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_node*           node  = create_fib_node(key, value);
    fib_heap_insert(fh, node);

    assert(fh->min == node);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_node*           s_node  = create_fib_node(s_key, s_value);
    fib_heap_insert(fh, s_node);

    assert(fh->min == s_node);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_node*           m_node  = create_fib_node(m_key, m_value);
    fib_heap_insert(fh, m_node);

    assert(fh->min == s_node);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_node*           r_node  = create_fib_node(r_key, r_value);
    fib_heap_insert(fh, r_node);

    assert(fh->min == r_node);

    fib_heap_destroy(fh);
}

void
test_fh_extract_min(void)
{
    fib_heap_t* fh = create_fib_heap();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_node*           node  = create_fib_node(key, value);
    fib_heap_insert(fh, node);

    assert(fh->min == node);
    assert(fh->num_nodes == 1);
    fib_node* e_node = fib_heap_extract_min(fh);
    assert(e_node == node);
    assert(fh->num_nodes == 0);
    assert(!fh->min);

    fib_heap_insert(fh, node);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_node*           s_node  = create_fib_node(s_key, s_value);
    fib_heap_insert(fh, s_node);

    assert(fh->min == s_node);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_node*           m_node  = create_fib_node(m_key, m_value);
    fib_heap_insert(fh, m_node);

    assert(fh->min == s_node);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_node*           r_node  = create_fib_node(r_key, r_value);
    fib_heap_insert(fh, r_node);

    assert(fh->min == r_node);

    e_node = fib_heap_extract_min(fh);
    assert(e_node == r_node);
    assert(fh->num_nodes == 3);
    assert(fh->min == s_node);

    assert(s_node->right == m_node);
    assert(s_node->left == m_node);
    assert(m_node->right == s_node);
    assert(m_node->left == s_node);

    assert(s_node->parent == NULL);
    assert(s_node->child == NULL);
    assert(m_node->parent == NULL);
    assert(m_node->child == node);
    assert(node->left == node);
    assert(node->right == node);
    assert(node->parent == m_node);
    assert(node->child == NULL);

    free(e_node);
    fib_heap_destroy(fh);
}

void
test_fh_union(void)
{
    fib_heap_t* fh  = create_fib_heap();
    fib_heap_t* fh1 = create_fib_heap();

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_node*           s_node  = create_fib_node(s_key, s_value);
    fib_heap_insert(fh, s_node);

    assert(fh->min == s_node);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_node*           m_node  = create_fib_node(m_key, m_value);
    fib_heap_insert(fh, m_node);

    assert(fh->min == s_node);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_node*           r_node  = create_fib_node(r_key, r_value);
    fib_heap_insert(fh1, r_node);

    fh = fib_heap_union(fh, fh1);

    assert(fh->num_nodes == 3);
    assert(fh->min == r_node);

    fib_heap_destroy(fh);
}

void
test_fh_decrease_key(void)
{
    fib_heap_t* fh = create_fib_heap();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_node*           node  = create_fib_node(key, value);
    fib_heap_insert(fh, node);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_node*           s_node  = create_fib_node(s_key, s_value);
    fib_heap_insert(fh, s_node);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_node*           m_node  = create_fib_node(m_key, m_value);
    fib_heap_insert(fh, m_node);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_node*           r_node  = create_fib_node(r_key, r_value);
    fib_heap_insert(fh, r_node);

    fib_heap_decrease_key(fh, r_node, 1);
    assert(fh->min == r_node);
    assert(fh->num_nodes == 4);

    const double new_min_key = 0.1;
    fib_heap_decrease_key(fh, m_node, new_min_key);
    assert(fh->min == m_node);
    assert(fh->num_nodes == 4);

    fib_heap_destroy(fh);
}

void
test_fh_delete(void)
{
    fib_heap_t* fh = create_fib_heap();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_node*           node  = create_fib_node(key, value);
    fib_heap_insert(fh, node);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_node*           s_node  = create_fib_node(s_key, s_value);
    fib_heap_insert(fh, s_node);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_node*           m_node  = create_fib_node(m_key, m_value);
    fib_heap_insert(fh, m_node);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_node*           r_node  = create_fib_node(r_key, r_value);
    fib_heap_insert(fh, r_node);

    fib_heap_delete(fh, s_node);
    assert(fh->num_nodes == 3);
    assert(fh->min == r_node);

    fib_heap_delete(fh, r_node);
    assert(fh->num_nodes == 2);
    assert(fh->min == m_node);

    fib_heap_destroy(fh);
}

int
main(void)
{
    test_create_fh();
    test_create_fh_node();
    test_fh_insert();
    test_fh_min();
    test_fh_extract_min();
    test_fh_union();
    test_fh_decrease_key();
    test_fh_delete();
}
