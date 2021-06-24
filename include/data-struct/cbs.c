#include "cbs.h"

#include <stdio.h>
#include <stdlib.h>

#include "access/node.h"
#include "access/relationship.h"

inline bool
ptr_eq(const void* a, const void* b)
{
    if (!a && !b) {
        return true;
    }
    if (!a || !b) {
        return false;
    }
    return a == b;
}

inline size_t
fnv_hash_ul(const unsigned long in, unsigned int seed)
{
    size_t             h     = seed;
    const unsigned int prime = 0xFDCFB7;

    return (h ^ in) * prime;
}

inline bool
unsigned_long_eq(const unsigned long a, const unsigned long b)
{
    return a == b;
}

inline void
unsigned_long_print(const unsigned long in)
{
    printf("%lu\n", in);
}

inline int
ul_cmp(const void* a, const void* b)
{
    unsigned long ai = *(unsigned long*)a;
    unsigned long bi = *(unsigned long*)b;

    return ai < bi ? -1 : ai == bi ? 0 : 1;
}

inline bool
int_eq(const int first, const int second)
{
    return first == second;
}

inline void
int_print(const int in)
{
    printf("%i\n", in);
}

inline bool
long_eq(const long first, const long second)
{
    return first == second;
}

inline void
long_print(const long in)
{
    printf("%li\n", in);
}

inline void
node_free(node_t* node)
{
    free(node);
}

inline void
rel_free(relationship_t* rel)
{
    free(rel);
}
