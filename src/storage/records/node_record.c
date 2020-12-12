#include "node_record.h"

node_t* new_node() {
    node_t *node;
    clear(node);
    return node;
}

int read(node_t* record, cursor_t* cursor) {
    // TODO
    return 0;
}

int write(const node_t* record, cursor_t* cursor) {
    // TODO
    return 0;
}

void clear(node_t *record) {
    record->id = UNINITIALIZED_LONG;
    record->flags = UNINITIALIZED_BYTE;
    record->first_relationship = UNINITIALIZED_LONG;
    record->first_property = UNINITIALIZED_LONG;
    record->node_type = UNINITIALIZED_LONG;
}

void copy(const node_t* original, node_t *copy) {
    copy->id = original->id;
    copy->flags = original->flags;
    copy->first_relationship = original->first_relationship;
    copy->first_property = original->first_property;
    copy->node_type = original->node_type;
}

bool equals(const node_t* first, const node_t* second) {
    return ((first->id == second->id)
            && (first->flags == second->flags)
            && (first->first_relationship == second->first_relationship)
            && (first->first_property == second->first_property)
            && (first->node_type == second->node_type));
}

int to_string(const node_t* record, char* buffer, size_t buffer_size) {
   int result = sprintf(buffer, "Node ID: %#lX\n"
                    "In-Use: %#hhX\n"
                    "First Relationship: %#lX\n"
                    "First Property: %#lX\n"
                    "Node Type: %#lX\n",
                    record->id,
                    record->flags,
                    record->first_relationship,
                    record->first_property,
                    record->node_type);

   if (result > buffer_size) {
       printf("Wrote relationship string representation to a buffer that was too small!");
       return EOVERFLOW;
   } else if (result < 0) {
       return result;
   }
    return 0;
}

