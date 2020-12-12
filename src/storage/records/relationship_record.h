#ifndef RELATIONSHIP_RECORD_H
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#include "../io/cursor.h"

#define UNINITIALIZED_LONG 0xFFFFFFFFFFFFFFFF
#define UNINITIALIZED_BYTE 0xFF


/**
 * The flags field is structured the floowoing way:
 * [       x] In Use
 * [      x ] First in chain of source node
 * [     x  ] First in chain of target node
 */
typedef struct RelationshipRecord {
    unsigned long int id;
    unsigned char flags;
    unsigned long int source_node;
    unsigned long int target_node;
    unsigned long int relationship_type;
    unsigned long int prev_rel_source;
    unsigned long int next_rel_source;
    unsigned long int prev_rel_target;
    unsigned long int next_rel_target;
    unsigned long int first_property;

    struct RelationshipRecord* (*new_relationship)();
    int (*read)(struct RelationshipRecord*, cursor_t*);
    int (*write)(const struct RelationshipRecord*, cursor_t*);
    void (*clear)(struct RelationshipRecord*);
    void (*copy)(const struct RelationshipRecord*, struct RelationshipRecord*);
    bool (*equals)(const struct RelationshipRecord*, const struct RelationshipRecord*);
    int (*to_string)(const struct RelationshipRecord*, char*, size_t);
} relationship_t;

/**
 *  Creates a new relationship struct and initializes it to FFFF.
 *
 *  @return: An empty relationship struct.
 */
relationship_t* new_relationship(void);

/*
 *  Reads the relationship relationship from disk beginning at the cursors position
 *  into a struct.
 *
 *  @param relationship: The struct to read the relationship into.
 *  @param cursor: Holds file and offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int read(relationship_t* relationship, unsigned long int id);

/**
 *  Writes the contents of the given relationship struct to the position of a
 *  file encoded in the cursor.
 *
 *  @param relationship: The struct to read the relationship into.
 *  @param cursor: Holds file and offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int write(const relationship_t* relationship, unsigned long int id);

/**
 * Clears the current relationship struct.
 *
 *  @param relationship: The relationship to be cleared.
 *
 *  @return: 0 on success, negative value otherwise.
 */
void clear(relationship_t* relationship);

/**
 * Copies the contents of the relationship.
 *
 * @param original: Record to be copied.
 * @param copy: Record to copy to.
 *
 * @return: 0 on success, negative value otherwise.
 */
void copy(const relationship_t* original, relationship_t* copy);

/**
 * Checks if two relationship structs have the same contents.
 *
 *  @param first: First node to compare against second.
 *  @param second: Other node to compare against first.
 */
bool equals(const relationship_t* first, const relationship_t* second);

/**
 *  Writes a string representation of the node relationship to a char buffer.
 *
 *  @param node: The node relationship to represent as string.
 *  @param buffer: Buffer of at least XYZ characters/bytes. TODO insert actual
 *
 *  @return: 0 on success, negative value on error.
 */
int to_string(const relationship_t* relationship, char* buffer, size_t buffer_size);
#endif
