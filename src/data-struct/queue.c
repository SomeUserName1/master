#include "queue.h"

#include <stdlib.h>
#include <string.h>

static bool
queue_passthrough_eq(const void* first, const void* second)
{
    return first == second;
}

static void*
queue_passthrough_copy(const void* elem)
{
    return (void*)elem;
}

static void
queue_passthrough_free(void* elem)
{
    elem = NULL;
}

queue_t*
create_queue(const queue_cbs_t* cbs)
{
    if (!cbs) {
        exit(-1);
    }

    queue_t* queue;
    queue = calloc(1, sizeof(*queue));

    if (!queue) {
        exit(-1);
    }

    queue->len  = 0;
    queue->head = NULL;
    queue->tail = NULL;

    memset(&(queue->cbs), 0, sizeof(queue->cbs));
    queue->cbs.qeq =
          cbs == NULL || cbs->qeq == NULL ? queue_passthrough_eq : cbs->qeq;
    queue->cbs.qcopy = cbs == NULL || cbs->qcopy == NULL
                             ? queue_passthrough_copy
                             : cbs->qcopy;
    queue->cbs.qfree = cbs == NULL || cbs->qfree == NULL
                             ? queue_passthrough_free
                             : cbs->qfree;

    return queue;
}

size_t
queue_size(queue_t* queue)
{
    if (queue == NULL) {
        return 0;
    }
    return queue->len;
}

void
queue_destroy(queue_t* queue)
{
    if (queue == NULL) {
        return;
    }

    queue_node_t* cur = queue->head;
    queue_node_t* next;

    for (size_t i = 0; i < queue->len; ++i) {
        queue->cbs.qfree(cur->element);
        next = cur->next;
        free(cur);
        cur = next;
    }

    free(queue);
}

int
queue_add(queue_t* queue, void* elem)
{
    if (queue == NULL || elem == NULL) {
        return -1;
    }

    queue_node_t* node = calloc(1, sizeof(*node));

    if (!node) {
        exit(-1);
    }

    node->element = queue->cbs.qcopy(elem);
    node->next    = NULL;
    node->prev    = NULL;

    if (queue->tail == NULL) {
        queue->head = node;
        queue->tail = node;

        node->next = node;
        node->prev = node;

        queue->len++;

        return 0;
    }

    node->prev = queue->tail;
    node->next = queue->head;

    queue->tail->next = node;
    queue->head->prev = node;

    queue->tail = node;
    queue->len++;

    return 0;
}

int
queue_insert(queue_t* queue, void* elem, size_t idx)
{
    if (queue == NULL || elem == NULL) {
        return -1;
    }

    queue_node_t* new = calloc(1, sizeof(*new));

    if (!new) {
        exit(-1);
    }

    new->element = queue->cbs.qcopy(elem);

    if (queue->len == 0 && idx == 0) {
        queue->head = new;
        queue->tail = new;

        new->prev = new;
        new->next = new;
        queue->len++;

        return 0;
    }

    if (idx >= queue->len) {
        free(new);
        return -1;
    }

    if (idx == 0 || idx == queue->len) {
        new->prev = queue->tail;
        new->next = queue->head;

        queue->head->prev = new;
        queue->tail->next = new;

        if (idx == 0) {
            queue->head = new;
        } else {
            queue->tail = new;
        }
    } else {
        queue_node_t* cur  = queue->head;
        queue_node_t* next = queue->head;

        for (size_t i = 0; i < idx; ++i) {
            cur  = next;
            next = cur->next;
        }

        new->next = next;
        new->prev = cur;

        cur->next  = new;
        next->prev = new;
    }

    queue->len++;

    return 0;
}

static int
queue_remove_int(queue_t* queue, size_t idx, bool free_flag)
{
    if (queue == NULL || idx >= queue->len) {
        return -1;
    }

    if (queue->len == 1 && idx == 0) {
        if (free_flag) {
            queue->cbs.qfree(queue->head->element);
        }
        free(queue->head);
        queue->head = NULL;
        queue->tail = NULL;
        queue->len--;

        return 0;
    }

    if (idx == 0) {
        if (free_flag) {
            queue->cbs.qfree(queue->head->element);
        }
        queue_node_t* temp = queue->head->next;
        free(queue->head);
        queue->head       = temp;
        queue->head->prev = NULL;
        queue->len--;

        return 0;
    }

    if (idx == queue->len - 1) {
        if (free_flag) {
            queue->cbs.qfree(queue->tail->element);
        }
        queue_node_t* temp = queue->tail->prev;
        free(queue->tail);
        queue->tail       = temp;
        queue->tail->next = NULL;
        queue->len--;

        return 0;
    }

    queue_node_t* cur;
    queue_node_t* next = queue->head;
    for (size_t i = 0; i < idx; ++i) {
        cur  = next;
        next = cur->next;
    }

    queue_node_t* remove = next;
    next                 = remove->next;
    cur->next            = next;
    next->prev           = cur;
    queue->len--;

    if (free_flag) {
        queue->cbs.qfree(remove->element);
    }
    free(remove);

    return 0;
}

int
queue_remove(queue_t* queue, size_t idx)
{
    return queue_remove_int(queue, idx, true);
}

int
queue_remove_elem(queue_t* queue, void* elem)
{
    size_t idx;
    if (queue_index_of(queue, elem, &idx) < 0) {
        return -1;
    }
    return queue_remove_int(queue, idx, true);
}

int
queue_index_of(queue_t* queue, void* elem, size_t* idx)
{
    if (queue == NULL || elem == NULL || idx == NULL || queue->len == 0) {
        return -1;
    }

    queue_node_t* cur = queue->head;
    for (size_t i = 0; i < queue->len; ++i) {
        if (queue->cbs.qeq(cur->element, elem)) {
            *idx = i;
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

bool
queue_contains(queue_t* queue, void* elem)
{
    size_t idx;
    return (queue_index_of(queue, elem, &idx) == 0);
}

void*
queue_get(queue_t* queue, size_t idx)
{
    if (queue == NULL || idx >= queue->len) {
        return NULL;
    }

    queue_node_t* cur = queue->head;
    for (size_t i = 0; i < idx; ++i) {
        cur = cur->next;
    }
    return cur->element;
}

void*
queue_take(queue_t* queue)
{
    void* result = queue->head->element;
    if (queue_remove_int(queue, 0, false) < 0) {
        return NULL;
    }
    return result;
}
