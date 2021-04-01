#ifndef FIBONACCI_HEAP
#define FIBONACCI_HEAP

#include <stdbool.h>

typedef struct fib_node
{
    double key;
    unsigned long value;
    struct fib_node* parent;
    struct fib_node* child;
    struct fib_node* left;
    struct fib_node* right;
    unsigned int degree;
    bool mark;
} fib_node;

typedef struct fib_heap
{
    fib_node* min;
    unsigned long num_nodes;
} fib_heap_t;

fib_node*
create_fib_node(double key, unsigned long value);

fib_heap_t*
create_fib_heap(void);

void
fib_heap_destroy(fib_heap_t* fh);

void
fib_heap_insert(fib_heap_t* fh, fib_node* node);

fib_node*
fib_heap_min(fib_heap_t* fh);

fib_node*
fib_heap_extract_min(fib_heap_t* fh);

fib_heap_t*
fib_heap_union(fib_heap_t* fh1, fib_heap_t* fh2);

void
fib_heap_decrease_key(fib_heap_t* fh, fib_node* node, double new_key);

void
fib_heap_delete(fib_heap_t* fh, fib_node* node);

#endif
