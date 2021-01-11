#include "list.h"

#include <stdlib.h>
#include <string.h>

static bool list_passthrough_eq(const void* first, const void* second) {
    return first == second;
}

list_t* create_list(list_cbs_t *cbs, list_flags_t flags) {
    list_t* list = malloc(sizeof(*list));
    list->elements = malloc(sizeof(*list->elements) * list_block_size);
    list->alloced = list_block_size;
    list->len = 0;
    list->inbulk = false;

    memset(&(list->cbs), 0, sizeof(list->cbs));
    if (cbs != NULL) {
        list->cbs.leq = cbs->leq;
        list->cbs.lcopy = cbs->lcopy;
        list->cbs.lfree = cbs->lfree;
    }

    if (list->cbs.leq == NULL) {
        list->cbs.leq = list_passthrough_eq;
    }

    list->flags = flags;

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

bool __binary_search_insert(const void *base, size_t nel, size_t width,
        const void *key, size_t *idx, bool is_insert,
        bool (*cmp)(const void *, const void *)) {
    size_t mid = 0;
    size_t left = 0;
    size_t right = 0;
    int eq  = -1;

    if (base == NULL || nel == 0 || key == NULL || idx == NULL || cmp == NULL) {
        return false;
    }

    left  = 0;
    right = nel;

    while (left < right) {
        mid = (left + right) / 2;
        eq  = cmp(&key, base + (mid * width));
        if (eq < 0) {
            right = mid;
        } else if (eq > 0) {
            left = mid+1;
        } else {
            break;
        }
    }

    if (is_insert) {
        if (eq > 0) {
            mid++;
        }

        while (mid < right && eq == 0) {
            mid++;
            eq = cmp(&key, base + (mid * width));
        }
    } else {
        if (eq != 0) {
            return false;
        }

        while (mid > 0 && mid >= left) {
            eq = cmp(&key, base + ((mid - 1) * width));
            if (eq != 0) {
                break;
            }
            mid--;
        }
    }

    *idx = mid;
    return true;
}

void* binary_search(const void* base, size_t nel, size_t width,
        const void* key, size_t* idx, bool (*cmp)(const void*, const void*)) {
    size_t  myidx = 0;

    if (idx == NULL) {
        idx = &myidx;
    }

    if (!__binary_search_insert(base, nel, width, key, idx, false, cmp)) {
        return NULL;
    }
    return (void *) base + (*idx * width);
}

int binary_insert(const void* base, size_t nel, size_t width,
        const void* key, size_t* idx, bool (*cmp)(const void*, const void*)) {

    if (!__binary_search_insert(base, nel, width, key, idx, true, cmp)) {
        return -1;
    }
    return 0;
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

    if (self->flags != LIST_NONE && !self->inbulk) {
        binary_insert(self->elements, self->len, sizeof(*self->elements), elem,
                &idx, self->cbs.leq);
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

    if (self->flags != LIST_SORT && !self->inbulk) {
       if (binary_search(self->elements, self->len, sizeof(*self->elements),
                   elem, idx, self->cbs.leq) < 0) {
        return -1;
       }
       return 0;
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
    size_t idx = 0;
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

int list_start_bulk_add(list_t* self) {
    if (self == NULL) {
        return -1;
    }
    self->inbulk = true;
    return 0;
}

int list_end_bulk_add(list_t* self) {
    if (self == NULL) {
        return -1;
    }
    self->inbulk = false;
    list_sort(self, NULL);
    return 0;
}

int merge_sort(void *base, size_t nel, size_t width,
        bool (*cmp)(const void *, const void *)) {
    if (base == NULL || nel < 2 || width == 0 || cmp == NULL) {
        return -1;
    }

    size_t mid = nel / 2;
    size_t ls  = mid;
    size_t rs  = mid;

    if (nel > 2 && nel % 2 != 0) {
        ls++;
    }

    char* left  = malloc(ls * width);
    char* right = malloc(rs * width);
    memcpy(left, base, ls * width);
    memcpy(right, base+(ls * width), rs * width);

    merge_sort(left, ls, width, cmp);
    merge_sort(right, rs, width, cmp);

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;
    while (i < ls && j < rs) {
        if (cmp(left + (i * width), right + (j * width)) <= 0) {
            memcpy(base + (k * width), left + (i * width), width);
            i++;
        } else {
            memcpy(base + (k * width), right + (j * width), width);
            j++;
        }
        k++;
    }

    while (i < ls) {
        memcpy(base + (k * width), left + (i * width), width);
        i++;
        k++;
    }

    while (j < rs) {
        memcpy(base + (k * width), right + (j * width), width);
        j++;
        k++;
    }

    free(right);
    free(left);
    return 0;
}

int list_sort(list_t* self , list_eq e) {
    if (self == NULL)
        return -1;

    if (self->flags & LIST_SORT) {
        if (e != NULL) {
            self->cbs.leq = e;
        } else {
            e = self->cbs.leq;
        }
    }

    if (e == NULL)
        return -1;

    merge_sort(self->elements, self->len, sizeof(*self->elements), e);
    return 0;
}
