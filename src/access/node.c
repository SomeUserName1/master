/*!
 * \file node.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref node.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "access/node.h"

#include <limits.h>
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
        // LCOV_EXCL_START
        printf("node - new node: failed to allocate memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    node_clear(node);
    return node;
}

inline void
node_free(node_t* node)
{
    if (!node) {
        // LCOV_EXCL_START
        printf("node - free: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    free(node);
}

void
node_read(node_t* record, page* read_from_page)
{
    if (!record || !read_from_page || read_from_page->pin_count < 1) {
        // LCOV_EXCL_START
        printf("node - node read: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t first_slot = (record->id & UCHAR_MAX) * SLOT_SIZE;

    record->first_relationship = read_ulong(read_from_page, first_slot);
    record->label =
          read_ulong(read_from_page, first_slot + sizeof(unsigned long));
}

void
node_write(node_t* record, page* write_to_page)
{
    if (!record || !write_to_page || write_to_page->pin_count < 1) {
        // LCOV_EXCL_START
        printf("node - node write: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    size_t first_slot = (record->id & UCHAR_MAX) * SLOT_SIZE;

    write_ulong(write_to_page, first_slot, record->first_relationship);
    write_ulong(
          write_to_page, first_slot + sizeof(unsigned long), record->label);
}

inline void
node_clear(node_t* record)
{
    if (!record) {
        // LCOV_EXCL_START
        printf("node - clear: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    record->id                 = UNINITIALIZED_LONG;
    record->first_relationship = UNINITIALIZED_LONG;
    record->label              = UNINITIALIZED_LONG;
}

inline node_t*
node_copy(const node_t* original)
{
    if (!original) {
        // LCOV_EXCL_START
        printf("node - copy: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    node_t* copy = malloc(sizeof(*copy));

    if (!copy) {
        // LCOV_EXCL_START
        printf("node - copy: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    copy->id                 = original->id;
    copy->first_relationship = original->first_relationship;
    copy->label              = original->label;

    return copy;
}

inline bool
node_equals(const node_t* first, const node_t* second)
{
    if (!first || !second) {
        // LCOV_EXCL_START
        printf("node - equals: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    return ((first->id == second->id)
            && (first->first_relationship == second->first_relationship)
            && (first->label == second->label));
}

void
node_to_string(const node_t* record, char* buffer, size_t buffer_size)
{
    if (!record || !buffer) {
        // LCOV_EXCL_START
        printf("node - to_string: Invalid Arguments\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    int length = snprintf(NULL,
                          0,
                          "Node ID: %lu\n"
                          "First Relationship: %lu\n"
                          "Label: %lu\n",
                          record->id,
                          record->first_relationship,
                          record->label);

    if (length < 0 || (size_t)length > buffer_size) {
        // LCOV_EXCL_START
        printf("Wrote node string representation to a buffer that was too "
               "small!");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }

    int result = snprintf(buffer,
                          length,
                          "Node ID: %lu\n"
                          "First Relationship: %lu\n"
                          "Label: %lu\n",
                          record->id,
                          record->first_relationship,
                          record->label);
    if (result < 0) {
        // LCOV_EXCL_START
        printf("node - node to string: failed to print string to buffer!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
}

void
node_pretty_print(const node_t* record)
{
    if (!record) {
        // LCOV_EXCL_START
        printf("node - pretty print: NULL pointer argument in pretty print "
               "node!\n");
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
    }
    printf("Node ID: %lu\n"
           "First Relationship: %lu\n"
           "Label: %lu\n",
           record->id,
           record->first_relationship,
           record->label);
}

ARRAY_LIST_IMPL(array_list_node, node_t*);
static array_list_node_cbs list_node_cbs = { node_equals, NULL, node_free };

inline array_list_node*
al_node_create(void)
{
    return array_list_node_create(list_node_cbs);
}

ARRAY_LIST_IMPL(inm_alist_node, node_t*);
static inm_alist_node_cbs inmal_node_cbs = { node_equals, NULL, NULL };

inline inm_alist_node*
inmal_node_create(void)
{
    return inm_alist_node_create(inmal_node_cbs);
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
