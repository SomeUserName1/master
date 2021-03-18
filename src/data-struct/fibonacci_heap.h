#ifndef FIBONACCI_HEAP
#define FIBONACCI_HEAP

#include <stdbool.h>

typedef struct fib_node {
    unsigned long key;
    void* value;
    struct fib_node* parent;
    struct fib_node* child;
    struct fib_node* left;
    struct fib_node* right;
    unsigned long degree;
    bool mark;
} fib_node;

typedef struct fib_heap {
    fib_node* min;
    unsigned int num_nodes;
} fib_heap_t;


fib_node* create_fib_node(unsigned long key, void* value);
fib_heap_t* create_fib_heap(void);
void destroy_fib_heap(fib_heap_t* fh);

int fib_heap_insert(fib_heap_t* fh, fib_node* node);
void* fib_heap_min(fib_heap_t* fh);
void* fib_heap_extract_min(fib_heap_t* fh);
fib_heap_t* fib_heap_union(fib_heap_t* fh1, fib_heap_t* fh2);
int fib_heap_decrease_key(fib_heap_t* fh, fib_node* node, unsigned long new_key);
int fib_heap_delete(fib_heap_t* fh, fib_node* node);

#endif
