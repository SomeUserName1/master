#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FIBONACCI_HEAP_DECL(typename, T)                                       \
    FIB_HEAP_STRUCTS(typename, T);                                             \
    typename* typename##_create(void);                                         \
    void typename##_destroy(typename* fh);                                     \
    void typename##_insert(typename* fh, double key, T value);                 \
    typename##_node* typename##_min(typename* fh);                             \
    void typename##_make_child(typename##_node* x, typename##_node* y);        \
    void typename##_consolidate(typename* fh);                                 \
    typename##_node* typename##_extract_min(typename* fh);                     \
    typename* typename##_union(typename* fh1, typename* fh2);                  \
    void typename##_cut(                                                       \
          typename* fh, typename##_node* node, typename##_node* parent);       \
    void typename##_cascading_cut(typename* fh, typename##_node* node);        \
    void typename##_decrease_key(                                              \
          typename* fh, typename##_node* node, double new_key);                \
    void typename##_delete(typename* fh, typename##_node* node);

#define FIBONACCI_HEAP_IMPL(typename, T)                                       \
    FIB_HEAP_CREATE(typename)                                                  \
    FIB_HEAP_DESTROY(typename)                                                 \
    FIB_HEAP_INSERT(typename, T)                                               \
    FIB_HEAP_MIN(typename)                                                     \
    FIB_HEAP_MAKE_CHILD(typename)                                              \
    FIB_HEAP_CONSOLIDATE(typename)                                             \
    FIB_HEAP_EXTRACT_MIN(typename)                                             \
    FIB_HEAP_UNION(typename)                                                   \
    FIB_HEAP_CUT(typename)                                                     \
    FIB_HEAP_CASCADING_CUT(typename)                                           \
    FIB_HEAP_DECREASE_KEY_INTERNAL(typename)                                   \
    FIB_HEAP_DECREASE_KEY(typename)                                            \
    FIB_HEAP_DELETE(typename)

#define FIB_HEAP_STRUCTS(typename, T)                                          \
    typedef struct fb_nd                                                       \
    {                                                                          \
        double        key;                                                     \
        T             value;                                                   \
        struct fb_nd* parent;                                                  \
        struct fb_nd* child;                                                   \
        struct fb_nd* left;                                                    \
        struct fb_nd* right;                                                   \
        unsigned int  degree;                                                  \
        bool          mark;                                                    \
    } typename##_node;                                                         \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        typename##_node* min;                                                  \
        unsigned long    num_nodes;                                            \
    } typename;

#define FIB_HEAP_CREATE(typename)                                              \
    typename* typename##_create(void)                                          \
    {                                                                          \
        typename* heap = malloc(sizeof(*heap));                                \
                                                                               \
        if (!heap) {                                                           \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        heap->min       = NULL;                                                \
        heap->num_nodes = 0;                                                   \
                                                                               \
        return heap;                                                           \
    }

#define FIB_HEAP_DESTROY(typename)                                             \
    void typename##_destroy(typename* fh)                                      \
    {                                                                          \
        if (fh->min) {                                                         \
            typename##_node* node;                                             \
            typename##_node* next = fh->min->right;                            \
                                                                               \
            /* iterate over all trees */                                       \
            for (size_t i = 0; i < fh->num_nodes; ++i) {                       \
                node = next;                                                   \
                                                                               \
                /* descend until the leaf */                                   \
                while (node->degree > 0) {                                     \
                    next = node->child;                                        \
                    node = next;                                               \
                }                                                              \
                                                                               \
                /* In a leaf, choose a sibling as next node to delete */       \
                if (node->right != node) {                                     \
                    next              = node->right;                           \
                    node->right->left = node->left;                            \
                    node->left->right = node->right;                           \
                } else if (node->parent) {                                     \
                    /* If there are no siblings in the leaf, check if the leaf \
                     * has parents */                                          \
                    next = node->parent;                                       \
                } else {                                                       \
                    /* if a leaf (no children) has neither siblings nor        \
                     * parents it's the last node to delete. This else         \
                     * statement is just to make static analysis happy,        \
                     * without it the free outside of this else block would be \
                     * executed and the loop would terminate                   \
                     * anyway (remember, last node). */                        \
                    free(node);                                                \
                    break;                                                     \
                }                                                              \
                                                                               \
                if (node->parent) {                                            \
                    node->parent->degree--;                                    \
                }                                                              \
                free(node);                                                    \
            }                                                                  \
        }                                                                      \
        free(fh);                                                              \
    }

#define FIB_HEAP_INSERT(typename, T)                                           \
    void typename##_insert(typename* fh, double key, T value)                  \
    {                                                                          \
        if (!fh) {                                                             \
            printf("fibonacci heap - insert: Invalid Argumentd!\n");           \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        if (key == -DBL_MAX) {                                                 \
            printf("A key of value DBL_MAX is not allowed!"                    \
                   "Please use somthing greater than"                          \
                   "the min value of the type as smallest priority!");         \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* node = malloc(sizeof(*node));                         \
                                                                               \
        if (!node) {                                                           \
            printf("fibonacci heap - fib_node_create: Memory Allocation "      \
                   "failed!\n");                                               \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        node->key    = key;                                                    \
        node->value  = value;                                                  \
        node->parent = NULL;                                                   \
        node->child  = NULL;                                                   \
        node->left   = NULL;                                                   \
        node->right  = NULL;                                                   \
        node->degree = 0;                                                      \
        node->mark   = false;                                                  \
                                                                               \
        if (!fh->min) {                                                        \
            fh->min     = node;                                                \
            node->left  = node;                                                \
            node->right = node;                                                \
        } else {                                                               \
            node->right       = fh->min->right;                                \
            node->right->left = node;                                          \
            node->left        = fh->min;                                       \
            fh->min->right    = node;                                          \
                                                                               \
            if (fh->min->key > node->key) {                                    \
                fh->min = node;                                                \
            }                                                                  \
        }                                                                      \
                                                                               \
        fh->num_nodes++;                                                       \
    }

#define FIB_HEAP_MIN(typename)                                                 \
    typename##_node* typename##_min(typename* fh)                              \
    {                                                                          \
        if (!fh) {                                                             \
            printf("fibonacci heap - min: Invalid Argumentd!\n");              \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
        if (!fh->min) {                                                        \
            printf("fibonacci_heap: minimum is not set.");                     \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
        return fh->min;                                                        \
    }

#define FIB_HEAP_MAKE_CHILD(typename)                                          \
    void typename##_make_child(typename##_node* x, typename##_node* y)         \
    {                                                                          \
        if (!x || !y) {                                                        \
            printf("fibonacci heap - make_child: Invalid Argumentd!\n");       \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        /* delete y from root list */                                          \
        y->left->right = y->right;                                             \
        y->right->left = y->left;                                              \
                                                                               \
        /* make y child of x */                                                \
        y->parent = x;                                                         \
        if (x->child) {                                                        \
            y->right        = x->child->right;                                 \
            y->right->left  = y;                                               \
            y->left         = x->child;                                        \
            x->child->right = y;                                               \
        } else {                                                               \
            x->child = y;                                                      \
            y->left  = y;                                                      \
            y->right = y;                                                      \
        }                                                                      \
        y->mark = false;                                                       \
        x->degree++;                                                           \
    }

#define FIB_HEAP_CONSOLIDATE(typename)                                         \
    static const double log_golden_ratio_factor = 2.1;                         \
    void typename##_consolidate(typename* fh)                                  \
    {                                                                          \
        if (!fh || !fh->min) {                                                 \
            printf("fibonacci heap - consolidate: Invalid Argumentd!\n");      \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
        unsigned int max_degree =                                              \
              floor(log_golden_ratio_factor * logf((float)fh->num_nodes));     \
        typename##_node** nodes_w_degree =                                     \
              calloc(max_degree, sizeof(typename##_node*));                    \
                                                                               \
        typename##_node* first = fh->min->right;                               \
        typename##_node* temp;                                                 \
        typename##_node* x = fh->min->right;                                   \
        typename##_node* y;                                                    \
        unsigned int     d;                                                    \
                                                                               \
        /* Collapse all nodes with the same degree until all degrees are       \
         * unique */                                                           \
        do {                                                                   \
            d = x->degree;                                                     \
                                                                               \
            /* Find roots with the same degree */                              \
            while (nodes_w_degree[d]) {                                        \
                y = nodes_w_degree[d];                                         \
                if (x == y) {                                                  \
                    break;                                                     \
                }                                                              \
                                                                               \
                /* Make the root with the smaller key a child of the other. */ \
                /* Clear mark, increment degree */                             \
                if (y->key < x->key) {                                         \
                    temp = x;                                                  \
                    x    = y;                                                  \
                    y    = temp;                                               \
                }                                                              \
                                                                               \
                typename##_make_child(x, y);                                   \
                                                                               \
                if (y == first) {                                              \
                    first = x;                                                 \
                }                                                              \
                                                                               \
                nodes_w_degree[d] = NULL;                                      \
                ++d;                                                           \
            }                                                                  \
            nodes_w_degree[d] = x;                                             \
            x                 = x->right;                                      \
        } while (x != first);                                                  \
                                                                               \
        /* rebuild root list */                                                \
        fh->min = NULL;                                                        \
                                                                               \
        for (size_t i = 0; i < max_degree; ++i) {                              \
            if (nodes_w_degree[i]) {                                           \
                x = nodes_w_degree[i];                                         \
                if (!fh->min) {                                                \
                    fh->min  = x;                                              \
                    x->left  = x;                                              \
                    x->right = x;                                              \
                } else {                                                       \
                    x->right             = fh->min->right;                     \
                    x->left              = fh->min;                            \
                    fh->min->right->left = x;                                  \
                    fh->min->right       = x;                                  \
                                                                               \
                    if (x->key < fh->min->key) {                               \
                        fh->min = x;                                           \
                    }                                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
        free(nodes_w_degree);                                                  \
    }

#define FIB_HEAP_EXTRACT_MIN(typename)                                         \
    typename##_node* typename##_extract_min(typename* fh)                      \
    {                                                                          \
        if (!fh) {                                                             \
            printf("fibonacci heap - consolidate: Invalid Argumentd!\n");      \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* z = fh->min;                                          \
        if (z) {                                                               \
            typename##_node* node = z->child;                                  \
            typename##_node* next;                                             \
                                                                               \
            /* iterate over all trees */                                       \
            while (node != NULL) {                                             \
                if (node->right != node) {                                     \
                    next = node->right;                                        \
                } else {                                                       \
                    next = NULL;                                               \
                }                                                              \
                                                                               \
                node->parent = NULL;                                           \
                                                                               \
                /* Remove node from min's children */                          \
                node->left->right = node->right;                               \
                node->right->left = node->left;                                \
                                                                               \
                /* Insert node to the right of min */                          \
                node->left     = z;                                            \
                node->right    = z->right;                                     \
                z->right->left = node;                                         \
                z->right       = node;                                         \
                                                                               \
                node = next;                                                   \
            }                                                                  \
                                                                               \
            /* Remove min from the root list. */                               \
            z->left->right = z->right;                                         \
            z->right->left = z->left;                                          \
                                                                               \
            if (z->right == z) {                                               \
                fh->min = NULL;                                                \
            } else {                                                           \
                typename##_consolidate(fh);                                    \
            }                                                                  \
            fh->num_nodes--;                                                   \
        } else {                                                               \
            printf("fibonacci_heap: minimum is not set.");                     \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
        return z;                                                              \
    }

#define FIB_HEAP_UNION(typename)                                               \
    typename* typename##_union(typename* fh1, typename* fh2)                   \
    {                                                                          \
        if (!fh1 || !fh2) {                                                    \
            printf("fibonacci heap - union: Invalid Argumentd!\n");            \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        typename* fh = typename##_create();                                    \
                                                                               \
        if (fh1->num_nodes == 0 && fh2->num_nodes == 0) {                      \
            return fh;                                                         \
        }                                                                      \
        if (fh1->num_nodes == 0 || fh2->num_nodes == 0) {                      \
            fh->min = fh1->num_nodes == 0 ? fh2->min : fh1->min;               \
        } else if (fh1->num_nodes > 0 && fh2->num_nodes > 0) {                 \
            typename##_node* fh2_temp;                                         \
            typename##_node* fh1_temp;                                         \
                                                                               \
            fh->min = fh1->min;                                                \
                                                                               \
            fh1_temp        = fh1->min->right;                                 \
            fh2_temp        = fh2->min->left;                                  \
            fh1->min->right = fh2->min;                                        \
            fh2->min->left  = fh1->min;                                        \
            fh1_temp->left  = fh2_temp;                                        \
            fh2_temp->right = fh1_temp;                                        \
                                                                               \
            if (fh2->min->key < fh->min->key) {                                \
                fh->min = fh2->min;                                            \
            }                                                                  \
        }                                                                      \
                                                                               \
        fh->num_nodes = fh1->num_nodes + fh2->num_nodes;                       \
                                                                               \
        free(fh1);                                                             \
        free(fh2);                                                             \
                                                                               \
        return fh;                                                             \
    }

#define FIB_HEAP_CUT(typename)                                                 \
    void typename##_cut(                                                       \
          typename* fh, typename##_node* node, typename##_node* parent)        \
    {                                                                          \
        if (!fh || !node || !parent) {                                         \
            printf("fibonacci heap - cut: Invalid Argumentd!\n");              \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        if (parent->degree == 1) {                                             \
            parent->child = NULL;                                              \
        } else {                                                               \
            node->right->left = node->left;                                    \
            node->left->right = node->right;                                   \
                                                                               \
            if (node == parent->child) {                                       \
                parent->child = node->right;                                   \
            }                                                                  \
        }                                                                      \
        parent->degree--;                                                      \
        node->parent = NULL;                                                   \
        node->mark   = false;                                                  \
                                                                               \
        node->right          = fh->min->right;                                 \
        node->left           = fh->min;                                        \
        fh->min->right->left = node;                                           \
        fh->min->right       = node;                                           \
    }

#define FIB_HEAP_CASCADING_CUT(typename)                                       \
    void typename##_cascading_cut(typename* fh, typename##_node* node)         \
    {                                                                          \
        if (!fh || !node) {                                                    \
            printf("fibonacci heap - cascading_cut: Invalid Argumentd!\n");    \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* parent;                                               \
                                                                               \
        while ((parent = node->parent)) {                                      \
            if (!node->mark) {                                                 \
                node->mark = true;                                             \
                break;                                                         \
            }                                                                  \
            typename##_cut(fh, node, parent);                                  \
            node = parent;                                                     \
        }                                                                      \
    }

#define FIB_HEAP_DECREASE_KEY_INTERNAL(typename)                               \
    void typename##_decrease_key_internal(                                     \
          typename* fh, typename##_node* node, double new_key, bool delete)    \
    {                                                                          \
        if (!fh || !node || new_key > node->key) {                             \
            printf("fib heap - decrease key: Arguments null or previous key "  \
                   "was "                                                      \
                   "larger then new key!\n");                                  \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        if (!delete &&new_key == -DBL_MAX) {                                   \
            printf("-DBL_MAX is reserved as priority!"                         \
                   "Please use somthing greater than"                          \
                   "that as smallest priority!");                              \
            exit(EXIT_FAILURE);                                                          \
        }                                                                      \
                                                                               \
        typename##_node* parent;                                               \
                                                                               \
        node->key = new_key;                                                   \
        parent    = node->parent;                                              \
                                                                               \
        if (parent && node->key < parent->key) {                               \
            typename##_cut(fh, node, parent);                                  \
            typename##_cascading_cut(fh, parent);                              \
        }                                                                      \
        if (node->key < fh->min->key) {                                        \
            fh->min = node;                                                    \
        }                                                                      \
    }

#define FIB_HEAP_DECREASE_KEY(typename)                                        \
    void typename##_decrease_key(                                              \
          typename* fh, typename##_node* node, double new_key)                 \
    {                                                                          \
        typename##_decrease_key_internal(fh, node, new_key, false);            \
    }

#define FIB_HEAP_DELETE(typename)                                              \
    void typename##_delete(typename* fh, typename##_node* node)                \
    {                                                                          \
        typename##_decrease_key_internal(fh, node, -DBL_MAX, true);            \
        free(typename##_extract_min(fh));                                      \
    }

FIBONACCI_HEAP_DECL(fib_heap_ul, unsigned long);

#endif
