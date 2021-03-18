#include "fibonacci_heap.h"

#include <stdlib.h>

fib_node* create_fib_node(unsigned long key, void* value) {
    fib_node* node = malloc(sizeof(*node));
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

fib_heap_t* create_fib_heap() {
    fib_heap_t* heap = malloc(sizeof(*heap));
    heap->min = NULL;
    heap->num_nodes = 0;

    return heap;
}

void destroy_fib_heap(fib_heap_t* fh) {
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
        }
        else {
            next = node->parent;
        }
        free(node);
        node = next;
    }
}

int fib_heap_insert(fib_heap_t* fh) {

}
