#include "access/relationship.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/array_list.h"
#include "data-struct/cbs.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "page.h"

inline relationship_t*
new_relationship()
{
    relationship_t* rel = malloc(sizeof(*rel));

    if (!rel) {
        exit(EXIT_FAILURE);
    }

    relationship_clear(rel);
    return rel;
}

void
relationship_read(relationship_t* record, page* read_from_page)
{
    if (!record || !read_from_page) {
        printf("relationship - read: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    size_t first_slot = record->id * ON_DISK_REL_SIZE % PAGE_SIZE;

    record->source_node = read_ulong(read_from_page, first_slot);
    record->target_node =
          read_ulong(read_from_page, first_slot + sizeof(unsigned long));

    record->prev_rel_source =
          read_ulong(read_from_page, first_slot + 2 * sizeof(unsigned long));
    record->next_rel_source =
          read_ulong(read_from_page, first_slot + 3 * sizeof(unsigned long));
    record->prev_rel_source =
          read_ulong(read_from_page, first_slot + 4 * sizeof(unsigned long));
    record->next_rel_target =
          read_ulong(read_from_page, first_slot + 5 * sizeof(unsigned long));

    record->weight =
          read_double(read_from_page, first_slot + 6 * sizeof(unsigned long));
    record->flags =
          read_uchar(read_from_page,
                     first_slot + 6 * sizeof(unsigned long) + sizeof(double));

    read_string(read_from_page,
                first_slot + 6 * sizeof(unsigned long) + sizeof(double)
                      + sizeof(unsigned char),
                record->label);
}

void
relationship_write(relationship_t* record, page* write_to_page)
{
    if (!record || !write_to_page) {
        printf("relationship - read: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    size_t first_slot = record->id * ON_DISK_REL_SIZE % PAGE_SIZE;

    write_ulong(write_to_page, first_slot, record->source_node);
    write_ulong(write_to_page,
                first_slot + sizeof(unsigned long),
                record->target_node);

    write_ulong(write_to_page,
                first_slot + 2 * sizeof(unsigned long),
                record->prev_rel_source);
    write_ulong(write_to_page,
                first_slot + 3 * sizeof(unsigned long),
                record->next_rel_source);
    write_ulong(write_to_page,
                first_slot + 4 * sizeof(unsigned long),
                record->prev_rel_source);
    write_ulong(write_to_page,
                first_slot + 5 * sizeof(unsigned long),
                record->next_rel_target);

    write_double(write_to_page,
                 first_slot + 6 * sizeof(unsigned long),
                 record->weight);
    write_uchar(write_to_page,
                first_slot + 6 * sizeof(unsigned long) + sizeof(double),
                record->flags);

    write_string(write_to_page,
                 first_slot + 6 * sizeof(unsigned long) + sizeof(double)
                       + sizeof(unsigned char),
                 record->label);
}

inline void
relationship_clear(relationship_t* record)
{
    record->id              = UNINITIALIZED_LONG;
    record->flags           = UNINITIALIZED_BYTE;
    record->source_node     = UNINITIALIZED_LONG;
    record->target_node     = UNINITIALIZED_LONG;
    record->prev_rel_source = UNINITIALIZED_LONG;
    record->next_rel_source = UNINITIALIZED_LONG;
    record->prev_rel_target = UNINITIALIZED_LONG;
    record->next_rel_target = UNINITIALIZED_LONG;
    record->weight          = UNINITIALIZED_WEIGHT;
    memset(record->label, 0, MAX_STR_LEN);
}

inline relationship_t*
relationship_copy(const relationship_t* original)
{
    relationship_t* copy = malloc(sizeof(*copy));

    if (copy == NULL) {
        return NULL;
    }

    copy->id              = original->id;
    copy->flags           = original->flags;
    copy->source_node     = original->source_node;
    copy->target_node     = original->target_node;
    copy->prev_rel_source = original->prev_rel_source;
    copy->next_rel_source = original->next_rel_source;
    copy->prev_rel_target = original->prev_rel_target;
    copy->next_rel_target = original->next_rel_target;
    copy->weight          = original->weight;

    return copy;
}

inline bool
relationship_equals(const relationship_t* first, const relationship_t* second)
{
    return ((first->id == second->id) && (first->flags == second->flags)
            && (first->source_node == second->source_node)
            && (first->target_node == second->target_node)
            && (first->prev_rel_source == second->prev_rel_source)
            && (first->next_rel_source == second->next_rel_source)
            && (first->prev_rel_target == second->prev_rel_target)
            && (first->weight == second->weight));
}

void
relationship_to_string(const relationship_t* record,
                       char*                 buffer,
                       size_t                buffer_size)
{
    int length = snprintf(NULL,
                          0,
                          "Relationship ID: %#lX\n"
                          "Flags: %#hhX\n"
                          "Source Node: %#lX\n"
                          "Target Node: %#lX\n"
                          "Source node's previous relationship: %#lX\n"
                          "Source node's next relationship: %#lX\n"
                          "Target node's previous relationship: %#lX\n"
                          "Target node's next relationship: %#lX\n"
                          "First Property: %.1f\n",
                          record->id,
                          record->flags,
                          record->source_node,
                          record->target_node,
                          record->prev_rel_source,
                          record->next_rel_source,
                          record->prev_rel_target,
                          record->next_rel_target,
                          record->weight);

    if (length < 0 || (size_t)length > buffer_size) {
        printf("Wrote relationship string representation to a buffer that was "
               "too small!\n");
        printf("String length: %lu, Buffer size, %lu\n",
               (size_t)length,
               buffer_size);
        exit(EXIT_FAILURE);
    }

    int result = snprintf(buffer,
                          length,
                          "Relationship ID: %#lX\n"
                          "Flags: %#hhX\n"
                          "Source Node: %#lX\n"
                          "Target Node: %#lX\n"
                          "Source node's previous relationship: %#lX\n"
                          "Source node's next relationship: %#lX\n"
                          "Target node's previous relationship: %#lX\n"
                          "Target node's next relationship: %#lX\n"
                          "First Property: %.1f\n",
                          record->id,
                          record->flags,
                          record->source_node,
                          record->target_node,
                          record->prev_rel_source,
                          record->next_rel_source,
                          record->prev_rel_target,
                          record->next_rel_target,
                          record->weight);

    if (result < 0) {
        printf("relationship - to string: Failed to write to buffer!\n");
        exit(EXIT_FAILURE);
    }
}

void
relationship_pretty_print(const relationship_t* record)
{
    printf("Relationship ID: %#lX\n"
           "Flags: %#hhX\n"
           "Source Node: %#lX\n"
           "Target Node: %#lX\n"
           "Source node's previous relationship: %#lX\n"
           "Source node's next relationship: %#lX\n"
           "Target node's previous relationship: %#lX\n"
           "Target node's next relationship: %#lX\n"
           "Weight: %.1f\n",
           record->id,
           record->flags,
           record->source_node,
           record->target_node,
           record->prev_rel_source,
           record->next_rel_source,
           record->prev_rel_target,
           record->next_rel_target,
           record->weight);
}

inline void
relationship_set_first_source(relationship_t* rel)
{
    if (!rel) {
        exit(EXIT_FAILURE);
    }

    if (rel->flags == UNINITIALIZED_BYTE) {
        rel->flags = FIRST_REL_SOURCE_FLAG;
    } else {
        rel->flags = rel->flags | FIRST_REL_SOURCE_FLAG;
    }
}

inline void
relationship_set_first_target(relationship_t* rel)
{
    if (!rel) {
        exit(EXIT_FAILURE);
    }

    if (rel->flags == UNINITIALIZED_BYTE) {
        rel->flags = FIRST_REL_TARGET_FLAG;
    } else {
        rel->flags = rel->flags | FIRST_REL_TARGET_FLAG;
    }
}

inline void
rel_free(relationship_t* rel)
{
    free(rel);
}

ARRAY_LIST_IMPL(array_list_relationship, relationship_t*);
array_list_relationship_cbs list_rel_cbs = { relationship_equals, NULL, NULL };

inline array_list_relationship*
al_rel_create(void)
{
    return array_list_relationship_create(list_rel_cbs);
}

HTABLE_IMPL(dict_ul_rel,
            unsigned long,
            relationship_t*,
            fnv_hash_ul,
            unsigned_long_eq);
dict_ul_rel_cbs d_rel_cbs = {
    NULL, NULL,     unsigned_long_print,      relationship_equals,
    NULL, rel_free, relationship_pretty_print
};

dict_ul_rel*
d_ul_rel_create(void)
{
    return dict_ul_rel_create(d_rel_cbs);
}

LINKED_LIST_IMPL(linked_list_relationship, relationship_t*);
linked_list_relationship_cbs ll_rel_cbs = { relationship_equals, NULL, NULL };

linked_list_relationship*
ll_rel_create(void)
{
    return linked_list_relationship_create(ll_rel_cbs);
}

