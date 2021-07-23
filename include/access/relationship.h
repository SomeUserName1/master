#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <stdbool.h>
#include <stddef.h>

#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "page.h"

#define ON_DISK_REL_SIZE                                                       \
    (6 * sizeof(unsigned long) + sizeof(double) + sizeof(unsigned char)        \
     + MAX_STR_LEN * sizeof(char))

#define FIRST_REL_SOURCE_FLAG (0x01)
#define FIRST_REL_TARGET_FLAG (0x02)

typedef enum
{
    OUTGOING = 0,
    INCOMING = 1,
    BOTH     = 2
} direction_t;

/**
 * The flags field is structured the floowoing way:
 * [       x] In Use
 * [      x ] First in chain of source node
 * [     x  ] First in chain of target node
 */
typedef struct RelationshipRecord
{
    unsigned long id;
    unsigned long source_node;
    unsigned long target_node;
    unsigned long prev_rel_source;
    unsigned long next_rel_source;
    unsigned long prev_rel_target;
    unsigned long next_rel_target;
    double        weight;
    unsigned char flags;
    char          label[MAX_STR_LEN];
} relationship_t;

/**
 *  Creates a new relationship struct and initializes it to FFFF.
 *
 *  @return: An empty relationship struct.
 */
relationship_t*
new_relationship(void);

/*
 *  Reads the relationship relationship from disk beginning at the given
 * address/id into a struct.
 *
 *  @param relationship: The struct to read the relationship into.
 *  @param id: Offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
void
relationship_read(relationship_t* record, page* read_from_page);

/**
 *  Writes the contents of the given relationship struct to the given
 * address/id.
 *
 *  @param relationship: The struct to read the relationship into.
 *  @param id: Offset to read from.
 *  @return: 0 on success, a negative int on failure.
 */
void
relationship_write(relationship_t* record, page* write_to_page);

/**
 * Clears the current relationship struct.
 *
 *  @param relationship: The relationship to be cleared.
 *
 *  @return: 0 on success, negative value otherwise.
 */
void
relationship_clear(relationship_t* record);

/**
 * Copies the contents of the relationship.
 *
 * @param original: Record to be copied.
 * @param copy: Record to copy to.
 *
 * @return: 0 on success, negative value otherwise.
 */
relationship_t*
relationship_copy(const relationship_t* original);

/**
 * Checks if two relationship structs have the same contents.
 *
 *  @param first: First node to compare against second.
 *  @param second: Other node to compare against first.
 */
bool
relationship_equals(const relationship_t* first, const relationship_t* second);

/**
 *  Writes a string representation of the node relationship to a char buffer.
 *
 *  @param node: The node relationship to represent as string.
 *  @param buffer: Buffer of at least XYZ characters/bytes. TODO insert actual
 *
 *  @return: 0 on success, negative value on error.
 */
void
relationship_to_string(const relationship_t* record,
                       char*                 buffer,
                       size_t                buffer_size);

void
relationship_pretty_print(const relationship_t* record);

void
relationship_set_first_source(relationship_t* rel);

void
relationship_set_first_target(relationship_t* rel);

void
rel_free(relationship_t* rel);

ARRAY_LIST_DECL(array_list_relationship, relationship_t*);

array_list_relationship*
al_rel_create(void);

HTABLE_DECL(dict_ul_rel, unsigned long, relationship_t*)

dict_ul_rel*
d_ul_rel_create(void);

LINKED_LIST_DECL(linked_list_relationship, relationship_t*);

linked_list_relationship*
ll_rel_create(void);

#endif
