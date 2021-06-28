#include "access/node.h"

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"

inline node_t*
new_node()
{
    node_t* node = malloc(sizeof(*node));

    if (!node) {
        exit(-1);
    }

    node_clear(node);
    return node;
}

int
node_read(node_t* record, const unsigned char* bytes)
{
    if (!record || !bytes) {
        exit(-1);
    }

    printf("%lu, %c", record->id, *bytes);
    return 0;
}

int
node_write(const node_t* record)
{
    if (!record) {
        exit(-1);
    }

    printf("%lu", record->id);
    return 0;
}

inline void
node_clear(node_t* record)
{
    if (!record) {
        exit(-1);
    }

    record->id                 = UNINITIALIZED_LONG;
    record->flags              = UNINITIALIZED_BYTE;
    record->first_relationship = UNINITIALIZED_LONG;
    record->degree             = 0;
}

inline node_t*
node_copy(const node_t* original)
{
    if (!original) {
        exit(-1);
    }

    node_t* copy = malloc(sizeof(*copy));

    if (!copy) {
        exit(-1);
    }

    copy->id                 = original->id;
    copy->flags              = original->flags;
    copy->first_relationship = original->first_relationship;
    copy->degree             = original->degree;

    return copy;
}

inline bool
node_equals(const node_t* first, const node_t* second)
{
    if (!first || !second) {
        return false;
    }

    return ((first->id == second->id) && (first->flags == second->flags)
            && (first->first_relationship == second->first_relationship)
            && first->degree == second->degree);
}

int
node_to_string(const node_t* record, char* buffer, size_t buffer_size)
{
    int length = snprintf(NULL,
                          0,
                          "Node ID: %#lX\n"
                          "In-Use: %#hhX\n"
                          "First Relationship: %#lX\n"
                          "Degree: %lu\n",
                          record->id,
                          record->flags,
                          record->first_relationship,
                          record->degree);

    if (length < 0 || (size_t)length > buffer_size) {
        printf("Wrote node string representation to a buffer that was too "
               "small!");
        return -1;
    }

    int result = snprintf(buffer,
                          length,
                          "Node ID: %#lX\n"
                          "In-Use: %#hhX\n"
                          "First Relationship: %#lX\n"
                          "Degree: %lu\n",
                          record->id,
                          record->flags,
                          record->first_relationship,
                          record->degree);

    return result > 0 ? 0 : result;
}

void
node_pretty_print(const node_t* record)
{
    if (!record) {
        printf("NULL pointer argument in pretty print node!\n");
        return;
    }
    printf("Node ID: %#lX\n"
           "In-Use: %#hhX\n"
           "First Relationship: %#lX\n"
           "Degree: %lu\n",
           record->id,
           record->flags,
           record->first_relationship,
           record->degree);
}

inline void
node_free(node_t* node)
{
    free(node);
}

ARRAY_LIST_IMPL(array_list_node, node_t*);
static array_list_node_cbs list_node_cbs = { node_equals, NULL, NULL };

inline array_list_node*
al_node_create(void)
{
    return array_list_node_create(list_node_cbs);
}

HTABLE_IMPL(dict_ul_node,
            unsigned long,
            node_t*,
            fnv_hash_ul,
            unsigned_long_eq);

dict_ul_node_cbs d_node_cbs = {
    NULL, NULL,      unsigned_long_print, node_equals,
    NULL, node_free, node_pretty_print
};

dict_ul_node*
d_ul_node_create(void)
{
    return dict_ul_node_create(d_node_cbs);
}

