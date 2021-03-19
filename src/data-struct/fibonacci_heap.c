#include "fibonacci_heap.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

fib_node*
create_fib_node(double key, unsigned long value)
{
    if (key == -DBL_MAX) {
        printf("A key of value DBL_MAX is not allowed!"
               "Please use somthing greater than"
               "-DBL_MAX as smallest priority!");
        exit(-1);
    }

    fib_node* node = malloc(sizeof(*node));

    if (!node) {
        exit(-1);
    }

    node->key = key;
    node->value = value;
    node->parent = NULL;
    node->child = NULL;
    node->left = NULL;
    node->right = NULL;
    node->degree = 0;
    node->mark = false;

    return node;
}

fib_heap_t*
create_fib_heap()
{
    fib_heap_t* heap = malloc(sizeof(*heap));

    if (!heap) {
        exit(-1);
    }

    heap->min = NULL;
    heap->num_nodes = 0;

    return heap;
}

void
destroy_fib_heap(fib_heap_t* fh)
{
    fib_node* node = fh->min->right;
    fib_node* next;

    /* iterate over all trees */
    while (node != NULL) {
        /* descend until the leaf */
        while (node->child != NULL) {
            node = node->child;
        }
        if (node->right != node) {
            next = node->right;
        } else {
            next = node->parent;
        }
        free(node);
        node = next;
    }
}

void
fib_heap_insert(fib_heap_t* fh, fib_node* node)
{
    if (!fh || !node) {
        exit(-1);
    }

    if (!fh->min) {
        fh->min = node;
        node->left = node;
        node->right = node;
    } else {
        node->right = fh->min->right;
        node->right->left = node;
        node->left = fh->min;
        fh->min->right = node;

        if (fh->min->key > node->key) {
            fh->min = node;
        }
    }
    fh->num_nodes++;
}

fib_node*
fib_heap_min(fib_heap_t* fh)
{
    if (!fh) {
        exit(-1);
    }
    if (!fh->min) {
        printf("fibonacci_heap: minimum is not set.");
    }
    return fh->min;
}

void
fib_heap_make_child(fib_node* x, fib_node* y)
{
    if (!x || !y) {
        exit(-1);
    }

    /* delete y from root list */
    y->left->right = y->right;
    y->right->left = y->left;

    /* make y child of x */
    y->parent = x;
    if (x->child) {
        y->right = x->child->right;
        y->left = x->child;
        x->child->right->left = y;
        x->child->right = y;
    } else {
        x->child = y;
        y->left = y;
        y->right = y;
    }
    y->mark = false;
    x->degree++;
}

void
fib_heap_consolidate(fib_heap_t* fh)
{
    if (!fh || !fh->min) {
        exit(-1);
    }

    int max_degree = 2 * (int)log((double)fh->num_nodes);
    fib_node** nodes_w_degree = calloc(max_degree, sizeof(fib_node*));

    fib_node* node = fh->min->right;
    fib_node* first = fh->min->right;
    fib_node* temp;
    fib_node* x;
    fib_node* y;
    unsigned int d;

    /* Collapse all nodes with the same degree until all degrees are unique */
    do {
        x = node;
        d = x->degree;
        /* Find roots with the same degree */
        while (nodes_w_degree[d]) {
            y = nodes_w_degree[d];

            /* Make the root with the smaller key a child of the other. */
            /* Clear mark, increment degree */
            if (y->key < x->key) {
                temp = x;
                x = y;
                y = temp;
            }
            fib_heap_make_child(x, y);
            nodes_w_degree[d] = NULL;
            ++d;
        }
        nodes_w_degree[d] = x;
        node = node->right;
    } while (node != first);

    /* rebuild root list */
    fh->min = NULL;

    for (size_t i = 0; i < max_degree; ++i) {
        if (nodes_w_degree[i]) {
            node = nodes_w_degree[i];
            if (!fh->min) {
                fh->min = node;
                node->left = node;
                node->right = node;
            } else {
                node->left = fh->min->left;
                node->right = fh->min;
                fh->min->left->right = node;
                fh->min->right = node;

                if (node->key < fh->min->key) {
                    fh->min = node;
                }
            }
        }
    }
}

fib_node*
fib_heap_extract_min(fib_heap_t* fh)
{
    if (!fh) {
        exit(-1);
    }

    fib_node* z = fh->min;
    if (z) {
        fib_node* node = z->child;
        fib_node* next;
        /* iterate over all trees */
        while (node != NULL) {

            if (node->right != node) {
                next = node->right;
            } else {
                next = NULL;
            }

            node->parent = NULL;
            node->left = z;
            node->right = z->right;
            z->right->left = node;
            z->right = node;

            node = next;
        }

        z->left->right = z->right;
        z->right->left = z->left;

        if (z->right == z) {
            fh->min = NULL;
        } else {
            fib_heap_consolidate(fh);
        }
        fh->num_nodes--;
    } else {
        printf("fibonacci_heap: minimum is not set.");
    }
    return z;
}

fib_heap_t*
fib_heap_union(fib_heap_t* fh1, fib_heap_t* fh2)
{
    if (!fh1 || !fh2) {
        exit(-1);
    }

    fib_heap_t* fh = create_fib_heap();

    if (fh1->num_nodes == 0 && fh2->num_nodes == 0) {
        return fh;
    }
    if (fh1->num_nodes == 0 || fh2->num_nodes == 0) {
        fh->min = fh1->num_nodes == 0 ? fh2->min : fh1->min;
    } else if (fh1->num_nodes > 0 && fh2->num_nodes > 0) {
        fib_node* fh2_temp;
        fib_node* fh1_temp;

        fh->min = fh1->min;

        fh1_temp = fh1->min->right;
        fh2_temp = fh2->min->left;
        fh1->min->right = fh2->min;
        fh2->min->left = fh1->min;
        fh1_temp->left = fh2_temp;
        fh2_temp->right = fh1_temp;

        if (fh2->min->key < fh->min->key) {
            fh->min = fh2->min;
        }
    }

    fh->num_nodes = fh1->num_nodes + fh2->num_nodes;

    return fh;
}

void
fib_heap_cut(fib_heap_t* fh, fib_node* node, fib_node* parent)
{
    if (!fh || !node || !parent) {
        exit(-1);
    }

    if (parent->degree == 1) {
        parent->child = NULL;
    } else {
        node->right->left = node->left;
        node->left->right = node->right;

        if (node == parent->child) {
            parent->child = node->right;
        }
    }
    parent->degree--;
    node->parent = NULL;
    node->mark = false;

    node->right = fh->min->right;
    node->left = fh->min;
    fh->min->right->left = node;
    fh->min->right = node;
}

void
fib_heap_cascading_cut(fib_heap_t* fh, fib_node* node)
{
    if (!fh || !node) {
        exit(-1);
    }

    fib_node* parent = node->parent;

    if (parent) {
        if (!node->mark) {
            node->mark = true;
        } else {
            fib_heap_cut(fh, node, parent);
            fib_heap_cascading_cut(fh, parent);
        }
    }
}

void
fib_heap_decrease_key_internal(fib_heap_t* fh,
                               fib_node* node,
                               double new_key,
                               bool delete)
{
    if (!fh || !node || new_key > node->key) {
        exit(-1);
    }

    if (new_key == -DBL_MAX) {
        printf("A key of value DBL_MAX is not allowed!"
               "Please use somthing greater than"
               "-DBL_MAX as smallest priority!");
        exit(-1);
    }

    if (delete) {
        new_key = 0;
    }

    fib_node* parent;

    node->key = new_key;
    parent = node->parent;

    if (parent && node->key < parent->key) {
        fib_heap_cut(fh, node, parent);
        fib_heap_cascading_cut(fh, parent);
    }
    if (node->key < fh->min->key) {
        fh->min = node;
    }
}

void
fib_heap_decrease_key(fib_heap_t* fh, fib_node* node, double new_key)
{
    fib_heap_decrease_key_internal(fh, node, new_key, false);
}

void
fib_heap_delete(fib_heap_t* fh, fib_node* node)
{
    fib_heap_decrease_key_internal(fh, node, -DBL_MAX, true);
    free(fib_heap_extract_min(fh));
}
