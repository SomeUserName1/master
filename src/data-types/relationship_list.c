#include "relationship_list.h"
#include "list.h"

bool __relationship_id_eq(const relationship_t* a,
        const relationship_t* b) {
    return a->id == b->id;
}

bool relationship_id_eq(const void* a, const void*b) {
    return __relationship_id_eq((relationship_t*) a, (relationship_t*) b);
}

bool relationship_eq(const void* a, const void* b) {
    return relationship_equals((relationship_t*) a, (relationship_t*) b);
}

void* relationship_cp(void* original) {
    relationship_t* copy = relationship_copy((relationship_t*) original);
    return (void*) copy;
}

void relationship_free(void* rel) {
    return;
}

relationship_list_t* create_relationship_list(rel_list_flags_t flags) {
    list_flags_t lflags = LIST_NONE;
    list_cbs_t cbs = {
        relationship_eq,
        relationship_cp,
        relationship_free
    };
    if (flags == REL_LIST_ID) {
        cbs.leq = relationship_id_eq;
    }
    list_t* lst =  create_list(&cbs, lflags);

    return (relationship_list_t*) lst;
}

