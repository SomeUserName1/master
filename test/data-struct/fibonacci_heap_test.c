#include "data-struct/fibonacci_heap.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void
test_create_fh(void)
{
    fib_heap_ul* fh = fib_heap_ul_create();

    assert(fh);
    assert(fh->num_nodes == 0);
    assert(!fh->min);

    fib_heap_ul_destroy(fh);
}

void
test_fh_insert(void)
{
    const double        key   = 42.42;
    const unsigned long value = 777;

    fib_heap_ul* fh = fib_heap_ul_create();

    fib_heap_ul_insert(fh, key, value);

    assert(fh->num_nodes == 1);
    assert(fh->min->key == key);
    assert(fh->min == fh->min->left);
    assert(fh->min == fh->min->right);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;

    fib_heap_ul_insert(fh, s_key, s_value);

    assert(fh->min->key == s_key);
    assert(fh->num_nodes == 2);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;

    fib_heap_ul_insert(fh, m_key, m_value);

    assert(fh->min->key == s_key);
    assert(fh->num_nodes == 3);
    assert(fh->min->right->key == m_key);
    assert(fh->min->right->right->key == key);
    assert(fh->min->right->right->right->key == s_key);
    assert(fh->min->left->key == key);
    assert(fh->min->left->left->key == m_key);
    assert(fh->min->left->left->left->key == s_key);
    fib_heap_ul_destroy(fh);
}

void
test_fh_min(void)
{
    fib_heap_ul* fh = fib_heap_ul_create();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_heap_ul_insert(fh, key, value);

    assert(fh->min->key == key);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_heap_ul_insert(fh, s_key, s_value);

    assert(fh->min->key == s_key);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_heap_ul_insert(fh, m_key, m_value);

    assert(fh->min->key == s_key);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_heap_ul_insert(fh, r_key, r_value);

    assert(fh->min->key == r_key);

    fib_heap_ul_destroy(fh);
}

void
test_fh_extract_min(void)
{
    fib_heap_ul* fh = fib_heap_ul_create();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_heap_ul_insert(fh, key, value);

    assert(fh->min->key == key);
    assert(fh->num_nodes == 1);
    fib_heap_ul_node* e_node = fib_heap_ul_extract_min(fh);
    assert(e_node->key == key);
    assert(fh->num_nodes == 0);
    assert(!fh->min);
    free(e_node);

    fib_heap_ul_insert(fh, key, value);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_heap_ul_insert(fh, s_key, s_value);

    assert(fh->min->key == s_key);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_heap_ul_insert(fh, m_key, m_value);

    assert(fh->min->key == s_key);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_heap_ul_insert(fh, r_key, r_value);

    assert(fh->min->key == r_key);

    e_node = fib_heap_ul_extract_min(fh);
    assert(e_node->key == r_key);
    assert(fh->num_nodes == 3);
    assert(fh->min->key == s_key);

    assert(fh->min->right->key == m_key);
    assert(fh->min->left->key == m_key);
    assert(fh->min->right->right == fh->min);
    assert(fh->min->left->left == fh->min);

    assert(fh->min->parent == NULL);
    assert(fh->min->child == NULL);
    assert(fh->min->right->parent == NULL);
    assert(fh->min->right->child->key == key);
    assert(fh->min->right->child->left->key == key);
    assert(fh->min->right->child->right->key == key);
    assert(fh->min->right->child->parent->key == m_key);
    assert(fh->min->right->child->child == NULL);

    free(e_node);
    fib_heap_ul_destroy(fh);
}

void
test_fh_union(void)
{
    fib_heap_ul* fh  = fib_heap_ul_create();
    fib_heap_ul* fh1 = fib_heap_ul_create();

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_heap_ul_insert(fh, s_key, s_value);

    assert(fh->min->key == s_key);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_heap_ul_insert(fh, m_key, m_value);

    assert(fh->min->key == s_key);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_heap_ul_insert(fh1, r_key, r_value);

    fh = fib_heap_ul_union(fh, fh1);

    assert(fh->num_nodes == 3);
    assert(fh->min->key == r_key);

    fib_heap_ul_destroy(fh);
}

void
test_fh_decrease_key(void)
{
    fib_heap_ul* fh = fib_heap_ul_create();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_heap_ul_insert(fh, key, value);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_heap_ul_insert(fh, s_key, s_value);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_heap_ul_insert(fh, m_key, m_value);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_heap_ul_insert(fh, r_key, r_value);

    fib_heap_ul_decrease_key(fh, fh->min, 1);
    assert(fh->min->key == 1);
    assert(fh->num_nodes == 4);

    const double new_min_key = 0.1;
    fib_heap_ul_decrease_key(fh, fh->min->left, new_min_key);
    assert(fh->min->key == new_min_key);
    assert(fh->num_nodes == 4);

    fib_heap_ul_destroy(fh);
}

void
test_fh_delete(void)
{
    fib_heap_ul* fh = fib_heap_ul_create();

    const double        key   = 42.42;
    const unsigned long value = 777;
    fib_heap_ul_insert(fh, key, value);

    const double        s_key   = 3.14159265;
    const unsigned long s_value = 11;
    fib_heap_ul_insert(fh, s_key, s_value);

    const double        m_key   = 9.14159265;
    const unsigned long m_value = 12;
    fib_heap_ul_insert(fh, m_key, m_value);

    const double        r_key   = 1.14159265;
    const unsigned long r_value = 14;
    fib_heap_ul_insert(fh, r_key, r_value);

    fib_heap_ul_delete(fh, fh->min);
    assert(fh->num_nodes == 3);

    fib_heap_ul_delete(fh, fh->min->right);
    assert(fh->num_nodes == 2);

    fib_heap_ul_destroy(fh);
}

int
main(void)
{
    test_create_fh();
    test_fh_insert();
    test_fh_min();
    test_fh_extract_min();
    test_fh_union();
    test_fh_decrease_key();
    test_fh_delete();
}
