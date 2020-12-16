#include "list.h"
#include <stdlib.h>
#include <string.h>

static int list_passthrough_eq(const void* first, const void* second) {
    return (*(void**) first) - (*(void**) second);
}

list_t* create_list(const list_cbs_t *cbs) {
    list_t* list;
    list = malloc(sizeof(*list));
    list->elements = malloc(sizeof(*list->elements) * list_block_size);
    list->alloced = list_block_size;
    list->len = 0;

    memset(&(list->cbs), 0, sizeof(list->cbs));
    if (cbs != NULL) {
        list->cbs.leq = cbs->leq;
        list->cbs.lcopy = cbs->lcopy;
        list->cbs.lfree = cbs->lfree;
    }

    if (list->cbs.leq == NULL) {
        list->cbs.leq = list_passthrough_eq;
    }

    list->destroy = list_destroy;
    list->size = list_size;
    list->add = list_add;
    list->insert = list_insert;
    list->remove = list_remove;
//    TODO here

    return list;
}

void list_destroy(list_t* self) {

}
