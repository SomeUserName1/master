#include "node_record.h"

node_record_t* new_node_record() {
    node_record_t *node;

    node->in_use = 0xFF;
    node->first_relationship = 0xFFFFFFFF;
    node->first_property = 0xFFFFFFFF;
    node->node_type = 0xFFFFFFFF;

    return node;
}


int read(node_record_t* record, cursor_t* cursor) {
    // TODO
    return 0;
}


int write(const node_record_t* record, cursor_t* cursor) {
    // TODO
    return 0;
}

void clear(node_record_t *record) {
    record->in_use = 0xFF;
    record->first_relationship = 0xFFFFFFFF;
    record->first_property = 0xFFFFFFFF;
    record->node_type = 0xFFFFFFFF;
}

void copy(const node_record_t* original, node_record_t *copy) {
    copy->in_use = original->in_use;
    copy->first_relationship = original->first_relationship;
    copy->first_property = original->first_property;
    copy->node_type = original->node_type;
}

bool equals(const node_record_t* first, const node_record_t* second) {
    return ((first->in_use == second->in_use)
            && (first->first_relationship == second->first_relationship)
            && (first->first_property == second->first_property)
            && (first->node_type == second->node_type));
}

int to_string(const node_record_t* record, char* buffer, size_t buffer_size) {
   if (buffer_size < NODE_RECORD_TO_STRING_LENGTH) {
        return EOVERFLOW;
   }
   sprintf(buffer, "In-Use: %#hhX\n"
                    "First Relationship: %#X\n"
                    "First Property: %#X\n"
                    "Node Type: %#X\n",
                    record->in_use,
                    record->first_relationship,
                    record->first_property,
                    record->node_type);
    return 0;
}

