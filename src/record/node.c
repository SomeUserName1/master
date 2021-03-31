#include "node.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"

node_t*
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

void
node_clear(node_t* record)
{
    if (!record) {
        exit(-1);
    }

    record->id = UNINITIALIZED_LONG;
    record->flags = UNINITIALIZED_BYTE;
    record->first_relationship = UNINITIALIZED_LONG;
    record->first_property = UNINITIALIZED_LONG;
    record->node_type = UNINITIALIZED_LONG;
}

node_t*
node_copy(const node_t* original)
{
    if (!original) {
        exit(-1);
    }

    node_t* copy = malloc(sizeof(*copy));

    if (!copy) {
        exit(-1);
    }

    copy->id = original->id;
    copy->flags = original->flags;
    copy->first_relationship = original->first_relationship;
    copy->first_property = original->first_property;
    copy->node_type = original->node_type;

    return copy;
}

bool
node_equals(const node_t* first, const node_t* second)
{
    if (!first || !second) {
        return false;
    }

    return ((first->id == second->id) && (first->flags == second->flags) &&
            (first->first_relationship == second->first_relationship) &&
            (first->first_property == second->first_property) &&
            (first->node_type == second->node_type));
}

int
node_to_string(const node_t* record, char* buffer, size_t buffer_size)
{
    int length = snprintf(NULL,
                          0,
                          "Node ID: %#lX\n"
                          "In-Use: %#hhX\n"
                          "First Relationship: %#lX\n"
                          "First Property: %#lX\n"
                          "Node Type: %#lX\n",
                          record->id,
                          record->flags,
                          record->first_relationship,
                          record->first_property,
                          record->node_type);

    if (length < 0 || (size_t)length > buffer_size) {
        printf("Wrote node string representation to a buffer that was too "
               "small!");
        return EOVERFLOW;
    }

    int result = snprintf(buffer,
                          length,
                          "Node ID: %#lX\n"
                          "In-Use: %#hhX\n"
                          "First Relationship: %#lX\n"
                          "First Property: %#lX\n"
                          "Node Type: %#lX\n",
                          record->id,
                          record->flags,
                          record->first_relationship,
                          record->first_property,
                          record->node_type);

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
           "First Property: %#lX\n"
           "Node Type: %#lX\n",
           record->id,
           record->flags,
           record->first_relationship,
           record->first_property,
           record->node_type);
}
