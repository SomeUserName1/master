#include "list.h"
#include <stdlib.h>
#include <string.h>

static bool list_passthrough_eq(const void* first, const void* second) {
    return (*(void**) first) - (*(void**) second) == 0 ? true : false;
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
    list->index_of = list_index_of;
    list->contains = list_contains;
    list->get = list_get;
    list->take = list_take;

    return list;
}

void list_destroy(list_t* self) {
    if (self == NULL) {
        return;
    }

    if (self->cbs.lfree != NULL) { 
        for (size_t i = 0; i < self->len; ++i)  {
            self->cbs.lfree(self->elements[i]);
        }
    }

    free(self->elements);
    free(self);
}

size_t list_size(list_t* self) {
    if (self == NULL) {
        return 0;
    }
    return self->len;
}

int list_append(list_t* self, void *elem) {
    if (self == NULL || elem == NULL) {
        return -1;
    }
    return list_insert(self, elem, self->len);
}

int list_insert(list_t* self, void* elem, size_t idx) {
    if (self == NULL || elem == NULL) {
        return -1;
    }

    if (self->alloced == self->len) {
        self->alloced += list_block_size;
        self->elements = realloc(self->elements,
                sizeof(*self->elements) * self->alloced);
        if (self->elements == NULL) {
            return -1;
        }
    }

    if (self->cbs.lcopy != NULL) {
        elem = self->cbs.lcopy(elem);
        if (elem == NULL) {
            return -1;
        }
    }

    if (idx > self->len) {
        idx = self->len;
    } else if (idx < self->len) {
       memmove(self->elements + idx + 1, self->elements + idx,
               (self->len - idx) * sizeof(*self->elements));
    }
    self->elements[idx] = elem;
    self->len++;

    return 0;
}

static int __list_remove(list_t* self, size_t idx, bool free_flag) {
    if (self == NULL || idx >= self->len) {
        return -1;
    }

    if (free_flag && self->cbs.lfree != NULL) {
        self->cbs.lfree(self->elements[idx]);
    }

    self->len--;
    if(idx == self->len) {
        return 0;
    }
    memmove(self->elements + idx, self->elements + idx + 1,
            (self->len - idx) * sizeof(*self->elements));
    return 0;
}

int list_remove(list_t* self, size_t idx) {
    return __list_remove(self, idx, true);
}

int list_index_of(list_t* self, void* elem, size_t* idx) {
    if (self == NULL || elem == NULL || idx == NULL) {
        return -1;
    }

    for (size_t i = 0; i < self->len; ++i) {
        if (self->cbs.leq(&elem, &self->elements[i]) == true) {
            *idx = i;
            return 0;
        }
    }
    return -1;
}

int list_remove_elem(list_t* self, void* elem) {
    size_t idx;
    if (list_index_of(self, elem, &idx) != 0) {
        return -1;
    }
    return __list_remove(self, idx, true);
}

bool list_contains(list_t* self, void* elem) {
    size_t idx = 0;
    return list_index_of(self, elem, &idx) == 0 ? true : false;
}

void* list_get(list_t* self, size_t idx) {
    if (self == NULL || idx >= self->len) {
        return NULL;
    }
    return self->elements[idx];
}

void* list_take(list_t* self, size_t idx) {
    void* elem = list_get(self, idx);
    __list_remove(self, idx, false);
    return elem;
}

