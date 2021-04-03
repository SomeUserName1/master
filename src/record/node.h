#ifndef NODE_H
#define NODE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * The struct that is stored on disk. The first byte is acutally ust a flag
 * but a byte is used to align the struct to be a aligned.
 *
 */
typedef struct NodeRecord
{
    unsigned long int id;
    unsigned char     flags;
    unsigned long int first_relationship;
    unsigned long int first_property;
    unsigned long int node_type;
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
int
node_read(node_t* record, const unsigned char* bytes);

/**
 *  Writes the contents of the given record struct to the given address/id.
 *
 *  @param record: The struct to read the record into.
 *  @param id: Offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int
node_write(const node_t* record);

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
int
node_to_string(const node_t* record, char* buffer, size_t buffer_size);

void
node_pretty_print(const node_t* record);

#endif
