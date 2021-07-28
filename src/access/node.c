#include "access/node.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "page.h"

inline node_t*
new_node()
{
    node_t* node = malloc(sizeof(*node));

    if (!node) {
        exit(EXIT_FAILURE);
    }

    node_clear(node);
    return node;
}

void
node_read(node_t* record, page* read_from_page)
{
    if (!record || !read_from_page) {
        printf("node - node read: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t first_slot = record->id * ON_DISK_NODE_SIZE % PAGE_SIZE;

    record->first_relationship = read_ulong(read_from_page, first_slot);
    read_string(
          read_from_page, first_slot + sizeof(unsigned long), record->label);
}

void
node_write(node_t* record, page* write_to_page)
{
    if (!record || !write_to_page) {
        printf("node - node write: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    size_t first_slot = record->id * ON_DISK_NODE_SIZE % PAGE_SIZE;

    write_ulong(write_to_page, first_slot, record->first_relationship);
    write_string(
          write_to_page, first_slot + sizeof(unsigned long), record->label);
}

inline void
node_clear(node_t* record)
{
    if (!record) {
        exit(EXIT_FAILURE);
    }

    record->id                 = UNINITIALIZED_LONG;
    record->first_relationship = UNINITIALIZED_LONG;
    memset(record->label, 0, MAX_STR_LEN);
}

inline node_t*
node_copy(const node_t* original)
{
    if (!original) {
        exit(EXIT_FAILURE);
    }

    node_t* copy = malloc(sizeof(*copy));

    if (!copy) {
        exit(EXIT_FAILURE);
    }

    copy->id                 = original->id;
    copy->first_relationship = original->first_relationship;
    strncpy(copy->label, original->label, MAX_STR_LEN);

    return copy;
}

inline bool
node_equals(const node_t* first, const node_t* second)
{
    if (!first || !second) {
        return false;
    }

    return ((first->id == second->id)
            && (first->first_relationship == second->first_relationship)
            && strncmp(first->label, second->label, MAX_STR_LEN) == 0);
}

void
node_to_string(const node_t* record, char* buffer, size_t buffer_size)
{
    int length = snprintf(NULL,
                          0,
                          "Node ID: %#lX\n"
                          "First Relationship: %#lX\n"
                          "Degree: %s\n",
                          record->id,
                          record->first_relationship,
                          record->label);

    if (length < 0 || (size_t)length > buffer_size) {
        printf("Wrote node string representation to a buffer that was too "
               "small!");
        exit(EXIT_FAILURE);
    }

    int result = snprintf(buffer,
                          length,
                          "Node ID: %#lX\n"
                          "First Relationship: %#lX\n"
                          "Label: %s\n",
                          record->id,
                          record->first_relationship,
                          record->label);
    if (result < 0) {
        printf("node - node to string: failed to print string to buffer!\n");
        exit(EXIT_FAILURE);
    }
}

void
node_pretty_print(const node_t* record)
{
    if (!record) {
        printf("NULL pointer argument in pretty print node!\n");
        return;
    }
    printf("Node ID: %#lX\n"
           "First Relationship: %#lX\n"
           "Label: %s\n",
           record->id,
           record->first_relationship,
           record->label);
}

inline void
node_free(node_t* node)
{
    free(node);
}

ARRAY_LIST_IMPL(array_list_node, node_t*);
static array_list_node_cbs list_node_cbs = { node_equals, NULL, node_free };

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

