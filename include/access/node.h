/*!
 * \file node.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief TODO
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef NODE_H
#define NODE_H

#include <stdbool.h>
#include <stddef.h>

#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "page.h"

#define ON_DISK_NODE_SIZE (sizeof(unsigned long) + sizeof(unsigned long))

#define NUM_SLOTS_PER_NODE                                                     \
    ((ON_DISK_NODE_SIZE / SLOT_SIZE) + (ON_DISK_NODE_SIZE % SLOT_SIZE != 0))

/**
 * The struct that is stored on disk. The first byte is acutally ust a flag
 * but a byte is used to align the struct to be a aligned.
 *
 */
typedef struct NodeRecord
{
    unsigned long id;
    unsigned long first_relationship;
    unsigned long label;
} node_t;

/**
 *  Creates a new record struct and initializes it to zero/null.
 *
 *  @return: An empty record struct.
 */
node_t*
new_node(void);

/*
 *  Reads the record from disk beginning at the given address/id into a
 *  struct.
 *
 *  @param record: The struct to read the record into.
 *  @param id: Offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
void
node_read(node_t* record, page* read_from_page);

/**
 *  Writes the contents of the given record struct to the given address/id.
 *
 *  @param record: The struct to read the record into.
 *  @param id: Offset to write to.
 *  @return: 0 on success, a negative int on failure.
 */
void
node_write(node_t* record, page* write_to_page);

/**
 * Clears the current record struct.
 *
 *  @param record: The record to be cleared.
 *
 *  @return: 0 on success, negative value otherwise.
 */
void
node_clear(node_t* record);

/**
 * Copies the contents of the record.
 *
 * @param original: Record to be copied.
 * @param copy: Record to copy to.
 *
 * @return: 0 on success, negative value otherwise.
 */
node_t*
node_copy(const node_t* original);

/**
 * Checks if two record structs have the same contents.
 *
 *  @param first: First node to compare against second.
 *  @param second: Other node to compare against first.
 */
bool
node_equals(const node_t* first, const node_t* second);

/**
 *  Writes a string representation of the node record to a char buffer.
 *
 *  @param node: The node record to represent as string.
 *  @param buffer: Buffer of at least 94 characters/bytes.
 *
 *  @return: 0 on success, negative value on error.
 */
void
node_to_string(const node_t* record, char* buffer, size_t buffer_size);

void
node_pretty_print(const node_t* record);

void
node_free(node_t* node);

ARRAY_LIST_DECL(array_list_node, node_t*);

array_list_node*
al_node_create(void);

ARRAY_LIST_DECL(inm_alist_node, node_t*);

inm_alist_node*
inmal_node_create(void);

HTABLE_DECL(dict_ul_node, unsigned long, node_t*);

dict_ul_node*
d_ul_node_create(void);

#endif
