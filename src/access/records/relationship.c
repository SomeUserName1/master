#include "relationship.h"
#include <stdlib.h>

relationship_t* new_relationship() {
   relationship_t *rel = malloc(sizeof(*rel));
   rel->clear(rel);
    return rel;
}

int relationship_read(relationship_t* record, unsigned long int id) {
    // TODO
    return 0;
}

int relationship_write(const relationship_t* record, unsigned long int id) {
    // TODO
    return 0;
}

void relationship_clear(relationship_t *rel) {
    rel->id = UNINITIALIZED_LONG;
    rel->flags = UNINITIALIZED_BYTE;
    rel->source_node = UNINITIALIZED_LONG;
    rel->target_node =  UNINITIALIZED_LONG;
    rel->relationship_type = UNINITIALIZED_LONG;
    rel->prev_rel_source = UNINITIALIZED_LONG;
    rel->next_rel_source = UNINITIALIZED_LONG;
    rel->prev_rel_target = UNINITIALIZED_LONG;
    rel->next_rel_target = UNINITIALIZED_LONG;
    rel->first_property = UNINITIALIZED_LONG;
}

relationship_t* relationship_copy(const relationship_t* original) {
    relationship_t* copy = malloc(sizeof(*copy));
    
    if (copy == NULL) {
        return NULL;
    }

    copy->id = original->id;
    copy->flags = original->flags;
    copy->source_node = original->source_node;
    copy->target_node =  original->target_node;
    copy->relationship_type = original->relationship_type;
    copy->prev_rel_source = original->prev_rel_source;
    copy->next_rel_source = original->next_rel_source;
    copy->prev_rel_target = original->prev_rel_target;
    copy->next_rel_target = original->next_rel_target;
    copy->first_property = original->first_property;

    return copy;
}

bool relationship_equals(const relationship_t* first, const relationship_t* second) {
    return ((first->id == second->id)
            && (first->flags == second->flags)
            && (first->source_node == second->source_node)
            && (first->target_node == second->target_node)
            && (first->relationship_type == second->relationship_type)
            && (first->prev_rel_source == second->prev_rel_source)
            && (first->next_rel_source == second->next_rel_source)
            && (first->prev_rel_target == second->prev_rel_target)
            && (first->first_property == second->first_property));
}

int relationship_to_string(const relationship_t* record, char* buffer, size_t buffer_size) {
   int result = sprintf(buffer, "Relationship ID: %#lX\n"
                    "Flags: %#hhX\n"
                    "Source Node: %#lX\n"
                    "Target Node: %#lX\n"
                    "Relationship Type: %#lX\n"
                    "Source node's previous relationship: %#lX\n"
                    "Source node's next relationship: %#lX\n"
                    "Target node's previous relationship: %#lX\n"
                    "Target node's next relationship: %#lX\n"
                    "First Property: %#lX\n",
                    record->id,
                    record->flags,
                    record->source_node,
                    record->target_node,
                    record->relationship_type,
                    record->prev_rel_source,
                    record->next_rel_source,
                    record->prev_rel_target,
                    record->next_rel_target,
                    record->first_property
          );
   if (result > buffer_size) {
       printf("Wrote relationship string representation to a buffer that was too small!");
       return EOVERFLOW;
   } else if (result < 0) {
       return result;
   }

    return 0;
}

