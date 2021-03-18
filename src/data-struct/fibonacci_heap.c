#include "fibonacci_heap.h"

#include <stdlib.h>

fib_node*
create_fib_node(unsigned long key, void* value)
{
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

int
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

    return 0;
}

void*
fib_heap_min(fib_heap_t* fh)
{
    if (!fh) {
        exit(-1);
    }
    return fh->min;
}

void*
fib_heap_extract_min(fib_heap_t* fh) {
    if (!fh) {
        exit(-1);
    }

    fib_node* z = fh->min;

    if (z) {
        // FIXME continue here with p.513 intro to algos, corman
    }
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
