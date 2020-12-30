#ifndef RELATIONSHIP_RECORD_H
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

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

    int (*read)(struct RelationshipRecord*, unsigned long int id);
    int (*write)(const struct RelationshipRecord*, unsigned long int id);
    void (*clear)(struct RelationshipRecord*);
    struct RelationshipRecord* (*copy)(const struct RelationshipRecord*);
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
 *  Reads the relationship relationship from disk beginning at the given address/id 
 *  into a struct.
 *
 *  @param relationship: The struct to read the relationship into.
 *  @param id: Offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int relationship_read(relationship_t* relationship, unsigned long int id);

/**
 *  Writes the contents of the given relationship struct to the given address/id.
 *
 *  @param relationship: The struct to read the relationship into.
 *  @param id: Offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
int relationship_write(const relationship_t* relationship, unsigned long int id);

/**
 * Clears the current relationship struct.
 *
 *  @param relationship: The relationship to be cleared.
 *
 *  @return: 0 on success, negative value otherwise.
 */
void relationship_clear(relationship_t* relationship);

/**
 * Copies the contents of the relationship.
 *
 * @param original: Record to be copied.
 * @param copy: Record to copy to.
 *
 * @return: 0 on success, negative value otherwise.
 */
relationship_t* relationship_copy(const relationship_t* original);

/**
 * Checks if two relationship structs have the same contents.
 *
 *  @param first: First node to compare against second.
 *  @param second: Other node to compare against first.
 */
bool relationship_equals(const relationship_t* first, const relationship_t* second);

/**
 *  Writes a string representation of the node relationship to a char buffer.
 *
 *  @param node: The node relationship to represent as string.
 *  @param buffer: Buffer of at least XYZ characters/bytes. TODO insert actual
 *
 *  @return: 0 on success, negative value on error.
 */
int relationship_to_string(const relationship_t* relationship, char* buffer, size_t buffer_size);
#endif
