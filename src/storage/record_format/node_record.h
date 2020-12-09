#include <stdbool.h>
#include <errno.h>

#include "../cursor.h"

/**
 * 2 for "0x" + 4 B * 2 hex digits per byte
 */
#define UINT_AS_HEX 12
/**
 * 8 B for "In-Use: " + 4 B for the byte as hex + 1B newline
 * 20 B for "First Relationship: " + UINT_AS_HEX for the address + 1B newline
 * 15 B for "First Property: " + UINT_AS_HEX for the address + 1 B newline
 * 10 B for "Node Type: " + UINT_AS_HEX + 1 for null terminal
 */
#define NODE_RECORD_TO_STRING_LENGTH (58 + 3 * UINT_AS_HEX)


/**
 * The struct that is stored on disk. The first byte is acutally ust a flag
 * but a byte is used to align the struct to be a aligned.
 *
 * TODO consider making un_use a int and pack the highest byte of relationship,
 * property and node type into the in use int => 5 bytes address space, no
 * wasted space, alginment to 2^4 B
 *
 */
typedef struct NodeRecord {
    unsigned char in_use;
    unsigned int first_relationship;
    unsigned int first_property;
    unsigned int node_type;

    struct NodeRecord* (*new_node_record)();
    int (*read)(struct NodeRecord*, cursor_t*);
    int (*write)(const struct NodeRecord*, cursor_t*);
    void (*clear)(struct NodeRecord*);
    void (*copy)(const struct NodeRecord*, struct NodeRecord*);
    bool (*equals)(const struct NodeRecord*, const struct NodeRecord*);
    int (*to_string)(const struct NodeRecord*, char*, size_t);
} node_record_t;

/**
 *  Creates a new record struct and initializes it to zero/null.
 *
 *  @return: An empty record struct.
 */
node_record_t* new_node_record(void);

/*
 *  Reads the record from disk beginning at the cursors position into a
 *  struct.
 *
 *  @param record: The struct to read the record into.
 *  @param cursor: Holds file and offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int read(node_record_t* record, cursor_t* cursor);

/**
 *  Writes the contents of the given record struct to the position of a
 *  file encoded in the cursor.
 *
 *  @param record: The struct to read the record into.
 *  @param cursor: Holds file and offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int write(const node_record_t* record, cursor_t* cursor);

/**
 * Clears the current record struct.
 *
 *  @param record: The record to be cleared.
 *
 *  @return: 0 on success, negative value otherwise.
 */
void clear(node_record_t* record);

/**
 * Copies the contents of the record.
 *
 * @param original: Record to be copied.
 * @param copy: Record to copy to.
 *
 * @return: 0 on success, negative value otherwise.
 */
void copy(const node_record_t* original, node_record_t* copy);

/**
 * Checks if two record structs have the same contents.
 *
 *  @param first: First node to compare against second.
 *  @param second: Other node to compare against first.
 */
bool equals(const node_record_t* first, const node_record_t* second);

/**
 *  Writes a string representation of the node record to a char buffer.
 *
 *  @param node: The node record to represent as string.
 *  @param buffer: Buffer of at least XYZ characters/bytes. TODO insert actual
 *
 *  @return: 0 on success, negative value on error.
 */
int to_string(const node_record_t* record, char* buffer, size_t buffer_size);
