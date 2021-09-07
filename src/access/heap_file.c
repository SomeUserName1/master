#include "access/heap_file.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "access/header_page.h"
#include "access/node.h"
#include "access/relationship.h"
#include "constants.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"

heap_file*
heap_file_create(page_cache* pc
#ifdef VERBOSE
                 ,
                 const char* log_path
#endif
)
{
    if (!pc
#ifdef VERBOSE
        || !log_path
#endif
    ) {
        printf("heap file - create: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    heap_file* hf          = malloc(sizeof(heap_file));
    hf->cache              = pc;
    hf->last_alloc_node_id = 0;
    hf->last_alloc_rel_id  = 0;
    hf->num_reads_nodes    = 0;
    hf->num_updates_nodes  = 0;
    hf->num_reads_rels     = 0;
    hf->num_update_rels    = 0;

    array_list_node* nodes = get_nodes(hf);
    hf->n_nodes            = array_list_node_size(nodes);
    array_list_node_destroy(nodes);

    array_list_relationship* rels = get_relationships(hf);
    hf->n_rels                    = array_list_relationship_size(rels);
    array_list_relationship_destroy(rels);

#ifdef VERBOSE
    FILE* log_file = fopen(log_path, "w+");

    if (log_file == NULL) {
        printf("heap file - create: Failed to open log file, %d\n", errno);
        exit(EXIT_FAILURE);
    }

    hf->log_file = log_file;
#endif

    return hf;
}

void
heap_file_destroy(heap_file* hf)
{
    if (!hf) {
        printf("heap_file - destroy: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

#ifdef VERBOSE
    fclose(hf->log_file);
#endif

    free(hf);
}

bool
check_record_exists(heap_file* hf, unsigned long id, bool node)
{
    if (!hf) {
        printf("heap file - check record exists: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned char slots = node ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;
    file_type     ft    = node ? node_ft : relationship_ft;

    size_t header_id   = (id * slots) / PAGE_SIZE * CHAR_BIT;
    page*  header_page = pin_page(hf->cache, header_id, header, ft);
    printf("rid %lu, hid %lu, hdata %u\n", id, header_id, header_page->data[0]);
    printf("offset %lu, hdata %u\n", header_id, header_page->data[0]);

    bool result = compare_bits(header_page->data,
                               PAGE_SIZE,
                               UCHAR_MAX,
                               (id * slots) % (PAGE_SIZE * CHAR_BIT),
                               slots);

    unpin_page(hf->cache, header_id, header, ft);

    return result;
}

void
next_free_slots(heap_file* hf, bool node)
{
    if (!hf) {
        printf("heap file - next free slots: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    file_type     ft      = node ? node_ft : relationship_ft;
    unsigned long n_slots = node ? NUM_SLOTS_PER_NODE : NUM_SLOTS_PER_REL;

    unsigned long* prev_allocd_id =
          node ? &(hf->last_alloc_node_id) : &(hf->last_alloc_rel_id);

    unsigned long cur_id = *prev_allocd_id;

    do {
        if (!check_record_exists(hf, cur_id, node)
            && (cur_id * n_slots) % SLOTS_PER_PAGE
                     < ((cur_id + 1) * n_slots - 1) % SLOTS_PER_PAGE) {
            *prev_allocd_id = cur_id;

            return;
        }

        cur_id++;
    } while (cur_id * n_slots / PAGE_SIZE
             >= hf->cache->pdb->header[ft]->num_pages);

    page* np        = new_page(hf->cache, ft);
    *prev_allocd_id = np->page_no * SLOTS_PER_PAGE / n_slots;
    unpin_page(hf->cache, np->page_no, records, ft);
}

static node_t*
read_node_internal(heap_file* hf, unsigned long node_id, bool check_exists)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        printf("heap file - read node internal: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (check_exists && !check_record_exists(hf, node_id, true)) {
        printf("heap file - read node internal: Node does not exist!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id   = node_id * NUM_SLOTS_PER_NODE / SLOTS_PER_PAGE;
    page*         node_page = pin_page(hf->cache, page_id, records, node_ft);

    node_t* node = new_node();
    node->id     = node_id;
    node_read(node, node_page);

    unpin_page(hf->cache, page_id, records, node_ft);
#ifdef VERBOSE
    fprintf(hf->log_file, "Read_Node %lu\n", node_id);
#endif

    hf->num_reads_nodes++;

    return node;
}

static relationship_t*
read_relationship_internal(heap_file*    hf,
                           unsigned long rel_id,
                           bool          check_exists)
{
    if (!hf || rel_id == UNINITIALIZED_LONG) {
        printf("heap file - read relationship internal: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (check_exists && !check_record_exists(hf, rel_id, false)) {
        printf("heap file - read relationship internal: Relationship does not "
               "exist!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id = rel_id * NUM_SLOTS_PER_REL / SLOTS_PER_PAGE;
    page* rel_page = pin_page(hf->cache, page_id, records, relationship_ft);

    relationship_t* rel = new_relationship();
    rel->id             = rel_id;
    relationship_read(rel, rel_page);

    unpin_page(hf->cache, page_id, records, relationship_ft);

#ifdef VERBOSE
    fprintf(hf->log_file, "read_rel %lu\n", rel_id);
#endif

    hf->num_reads_rels++;

    return rel;
}

static void
update_node_internal(heap_file* hf, node_t* node_to_write, bool check_exists)
{
    if (!hf || !node_to_write || node_to_write->id == UNINITIALIZED_LONG) {
        printf("heap file - update node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (check_exists && !check_record_exists(hf, node_to_write->id, true)) {
        printf("heap file - update node internal: Node does not exist!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id =
          node_to_write->id * NUM_SLOTS_PER_NODE / SLOTS_PER_PAGE;
    page* node_page = pin_page(hf->cache, page_id, records, node_ft);

    node_write(node_to_write, node_page);

    unpin_page(hf->cache, page_id, records, node_ft);

#ifdef VERBOSE
    fprintf(hf->log_file, "update_node %lu\n", node_to_write->id);
#endif

    hf->num_updates_nodes++;
}

static void
update_relationship_internal(heap_file*      hf,
                             relationship_t* rel_to_write,
                             bool            check_exists)
{
    if (!hf || !rel_to_write || rel_to_write->id == UNINITIALIZED_LONG
        || rel_to_write->source_node == UNINITIALIZED_LONG
        || rel_to_write->target_node == UNINITIALIZED_LONG
        || rel_to_write->weight == UNINITIALIZED_WEIGHT
        || rel_to_write->flags == UNINITIALIZED_BYTE) {
        printf("heap file - update node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (check_exists && !check_record_exists(hf, rel_to_write->id, false)) {
        printf(
              "heap file - update relationship internal: Relationship does not "
              "exist!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long page_id =
          rel_to_write->id * NUM_SLOTS_PER_REL / SLOTS_PER_PAGE;
    page* rel_page = pin_page(hf->cache, page_id, records, relationship_ft);

    relationship_t* rel = new_relationship();

    relationship_write(rel, rel_page);

    unpin_page(hf->cache, page_id, records, relationship_ft);

#ifdef VERBOSE
    fprintf(hf->log_file, "update_rel %lu\n", rel_to_write->id);
#endif

    hf->num_update_rels++;
}

unsigned long
create_node(heap_file* hf, char* label)
{
    if (!hf || !label) {
        printf("heap file - create node: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }

    next_free_slots(hf, true);
    unsigned long node_id = hf->last_alloc_node_id;

    node_t* node = new_node();
    node->id     = node_id;
    strncpy(node->label, label, MAX_STR_LEN);

    update_node_internal(hf, node, false);

    free(node);

    unsigned long header_id =
          ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) / PAGE_SIZE;

    page* header_page = pin_page(hf->cache, header_id, header, node_ft);

    unsigned short byte_offset =
          ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;

    unsigned char  bit_offset = (node_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char* used_bits  = malloc(sizeof(unsigned char));
    used_bits[0]              = UCHAR_MAX;

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_NODE,
               used_bits);

    unpin_page(hf->cache, header_id, header, node_ft);

    hf->n_nodes++;

    return node_id;
}

unsigned long
create_relationship(heap_file*    hf,
                    unsigned long from_node_id,
                    unsigned long to_node_id,
                    double        weight,
                    char*         label)
{
    if (!hf || from_node_id == UNINITIALIZED_LONG
        || to_node_id == UNINITIALIZED_LONG || weight == UNINITIALIZED_WEIGHT
        || !label) {
        printf("heap file - create relationship: Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }
    next_free_slots(hf, false);
    unsigned long rel_id = hf->last_alloc_rel_id;

    relationship_t* rel = new_relationship();
    rel->id             = rel_id;
    strncpy(rel->label, label, MAX_STR_LEN);

    node_t* from_node = read_node_internal(hf, from_node_id, true);
    node_t* to_node   = read_node_internal(hf, from_node_id, true);

    relationship_t* first_rel_from;
    relationship_t* last_rel_from;
    relationship_t* first_rel_to;
    relationship_t* last_rel_to;
    unsigned long   temp_id;

    if (from_node->first_relationship == UNINITIALIZED_LONG) {
        first_rel_from = rel;
        last_rel_from  = rel;
    } else {
        first_rel_from = read_relationship(hf, from_node->first_relationship);
        temp_id        = from_node->id == first_rel_from->source_node
                               ? first_rel_from->prev_rel_source
                               : first_rel_from->prev_rel_target;

        last_rel_from = read_relationship(hf, temp_id);
    }

    if (to_node->first_relationship == UNINITIALIZED_LONG) {
        first_rel_to = rel;
        last_rel_to  = rel;
    } else {
        first_rel_to = read_relationship(hf, to_node->first_relationship);
        temp_id      = from_node->id == first_rel_to->source_node
                             ? first_rel_to->prev_rel_source
                             : first_rel_to->prev_rel_target;

        last_rel_to = read_relationship(hf, temp_id);
    }

    rel->prev_rel_source = last_rel_from->id;
    rel->next_rel_source = first_rel_from->id;
    rel->prev_rel_target = last_rel_to->id;
    rel->prev_rel_target = first_rel_to->id;

    // Adjust next pointer in source node's previous relation
    if (last_rel_from->source_node == from_node_id) {
        last_rel_from->next_rel_source = rel->id;
    }
    if (last_rel_from->target_node == from_node_id) {
        last_rel_from->next_rel_target = rel->id;
    }
    // Adjust next pointer in target node's previous relation
    if (last_rel_to->source_node == to_node_id) {
        last_rel_to->next_rel_source = rel->id;
    } else {
        last_rel_to->next_rel_target = rel->id;
    }

    // Adjust previous pointer in source node's next relation
    if (first_rel_from->source_node == from_node_id) {
        first_rel_from->prev_rel_source = rel->id;
    } else {
        first_rel_from->prev_rel_target = rel->id;
    }
    // Adjust previous pointer in target node's next relation
    if (first_rel_to->source_node == to_node_id) {
        first_rel_to->prev_rel_source = rel->id;
    } else {
        first_rel_to->prev_rel_target = rel->id;
    }

    // Set the first relationship pointer, if the inserted rel is the first rel
    if (from_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_source(rel);
        from_node->first_relationship = rel->id;
        update_node(hf, from_node);
    }

    if (to_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_target(rel);
        to_node->first_relationship = rel->id;
        update_node(hf, to_node);
    }

    update_relationship_internal(hf, rel, false);
    update_relationship_internal(hf, first_rel_from, false);
    update_relationship_internal(hf, last_rel_from, false);
    update_relationship_internal(hf, first_rel_to, false);
    update_relationship_internal(hf, last_rel_to, false);

    free(rel);

    // When we have two relationships in the chain, the first and the last rel
    // are the same
    if (first_rel_from != last_rel_from) {
        free(first_rel_from);
    }
    // When we have only the newly added relationship in the chain last = first
    // = rel
    if (last_rel_from != rel) {
        free(last_rel_from);
    }

    if (first_rel_to != last_rel_to) {
        free(first_rel_to);
    }
    if (last_rel_to != rel) {
        free(last_rel_to);
    }

    unsigned long header_id =
          ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) / PAGE_SIZE;

    page* header_page = pin_page(hf->cache, header_id, header, relationship_ft);

    unsigned short byte_offset =
          ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;

    unsigned char  bit_offset = (rel_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char* used_bits  = malloc(sizeof(unsigned char));
    used_bits[0]              = UCHAR_MAX;

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_REL,
               used_bits);

    unpin_page(hf->cache, header_id, header, node_ft);

    hf->n_rels++;

    return rel_id;
}

node_t*
read_node(heap_file* hf, unsigned long node_id)
{
    return read_node_internal(hf, node_id, true);
}

relationship_t*
read_relationship(heap_file* hf, unsigned long rel_id)
{
    return read_relationship_internal(hf, rel_id, true);
}

void
update_node(heap_file* hf, node_t* node_to_write)
{
    update_node_internal(hf, node_to_write, true);
}

void
update_relationship(heap_file* hf, relationship_t* rel_to_write)
{
    update_relationship_internal(hf, rel_to_write, true);
}

void
delete_node(heap_file* hf, unsigned long node_id)
{
    if (!hf || node_id == UNINITIALIZED_LONG) {
        printf("heap file - delete node: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = read_node_internal(hf, node_id, true);

    if (node->first_relationship != UNINITIALIZED_LONG) {
        relationship_t* rel = read_relationship(hf, node->first_relationship);
        unsigned long   prev_rel_id = node_id == rel->source_node
                                            ? rel->prev_rel_source
                                            : rel->prev_rel_target;

        while (prev_rel_id != UNINITIALIZED_LONG) {
            rel         = read_relationship(hf, prev_rel_id);
            prev_rel_id = node_id == rel->source_node ? rel->prev_rel_source
                                                      : rel->prev_rel_target;
            delete_relationship(hf, rel->id);
        }
    }

#ifdef VERBOSE
    fprintf(hf->log_file, "delete_node %lu\n", node_id);
#endif

    unsigned long header_id =
          ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) / PAGE_SIZE;

    page* header_page = pin_page(hf->cache, header_id, header, node_ft);

    unsigned short byte_offset =
          ((node_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;
    unsigned char bit_offset  = (node_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char unused_bits = 0;

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_NODE,
               &unused_bits);

    unpin_page(hf->cache, header_id, header, node_ft);

    if (node_id < hf->last_alloc_node_id) {
        hf->last_alloc_node_id = node_id;
    }

    hf->n_nodes--;
}

void
delete_relationship(heap_file* hf, unsigned long rel_id)
{
    if (!hf || rel_id == UNINITIALIZED_LONG) {
        printf("heap file - delete relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    relationship_t* rel = read_relationship(hf, rel_id);

    // Adjust next pointer in source node's previous relation
    relationship_t* prev_rel_from = read_relationship(hf, rel->prev_rel_source);
    if (prev_rel_from->source_node == rel->source_node) {
        prev_rel_from->next_rel_source = rel->next_rel_source;
    } else {
        prev_rel_from->next_rel_target = rel->next_rel_source;
    }

    // Adjust next pointer in target node's previous relation
    relationship_t* prev_rel_to = read_relationship(hf, rel->prev_rel_target);
    if (prev_rel_to->source_node == rel->target_node) {
        prev_rel_to->next_rel_source = rel->next_rel_target;
    } else {
        prev_rel_to->next_rel_target = rel->next_rel_target;
    }

    // Adjust previous pointer in source node's next relation
    relationship_t* next_rel_from = read_relationship(hf, rel->next_rel_source);
    if (next_rel_from->source_node == rel->source_node) {
        next_rel_from->prev_rel_source = rel->prev_rel_source;
    } else {
        next_rel_from->prev_rel_target = rel->prev_rel_source;
    }

    // Adjust previous pointer in target node's next relation
    relationship_t* next_rel_to = read_relationship(hf, rel->next_rel_target);
    if (next_rel_to->source_node == rel->target_node) {
        next_rel_to->prev_rel_source = rel->prev_rel_target;
    } else {
        next_rel_to->prev_rel_target = rel->prev_rel_target;
    }

    update_relationship(hf, prev_rel_from);
    update_relationship(hf, prev_rel_to);
    update_relationship(hf, next_rel_from);
    update_relationship(hf, next_rel_to);

    node_t* node;
    if ((rel->flags & FIRST_REL_SOURCE_FLAG) != 0) {
        node = read_node_internal(hf, rel->source_node, true);
        node->first_relationship = rel->next_rel_source;
        update_node(hf, node);

        rel = read_relationship(hf, rel->next_rel_source);
        relationship_set_first_source(rel);
        update_relationship(hf, rel);
    }

    if ((rel->flags & FIRST_REL_TARGET_FLAG) != 0) {
        node = read_node_internal(hf, rel->target_node, true);
        node->first_relationship = rel->next_rel_target;
        update_node(hf, node);

        rel = read_relationship(hf, rel->next_rel_target);
        relationship_set_first_target(rel);
        update_relationship(hf, rel);
    }

#ifdef VERBOSE
    fprintf(hf->log_file, "delete_rel %lu\n", rel_id);
#endif

    unsigned long header_id =
          ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) / PAGE_SIZE;

    page* header_page = pin_page(hf->cache, header_id, header, relationship_ft);

    unsigned short byte_offset =
          ((rel_id * NUM_SLOTS_PER_NODE) / CHAR_BIT) % PAGE_SIZE;

    unsigned char bit_offset  = (rel_id * NUM_SLOTS_PER_NODE) % CHAR_BIT;
    unsigned char unused_bits = 0;

    write_bits(hf->cache,
               header_page,
               byte_offset,
               bit_offset,
               NUM_SLOTS_PER_REL,
               &unused_bits);

    unpin_page(hf->cache, header_id, header, node_ft);

    if (rel_id < hf->last_alloc_rel_id) {
        hf->last_alloc_rel_id = rel_id;
    }

    hf->n_rels--;
}

array_list_node*
get_nodes(heap_file* hf)
{
    if (!hf) {
        printf("heap file operators - get nodes: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    page*         catalogue_page = pin_page(hf->cache, 0, catalogue, 0);
    unsigned long n_slots        = read_ulong(catalogue_page, 0);
    unpin_page(hf->cache, 0, catalogue, 0);

    array_list_node* result = al_node_create();
    node_t*          node;

    for (size_t i = 0; i < n_slots / NUM_SLOTS_PER_NODE; i++) {
        // If the slot is used, load the node and append it to the resulting
        // array list
        if (check_record_exists(hf, i, true)) {
            node = read_node_internal(hf, i, false);
            array_list_node_append(result, node);
        }
    }

    return result;
}

array_list_relationship*
get_relationships(heap_file* hf)
{
    if (!hf) {
        printf("heap file - get relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    page*         catalogue_page = pin_page(hf->cache, 0, catalogue, 0);
    unsigned long n_slots = read_ulong(catalogue_page, sizeof(unsigned long));
    unpin_page(hf->cache, 0, catalogue, 0);

    array_list_relationship* result = al_rel_create();
    relationship_t*          rel;
    for (size_t i = 0; i < n_slots / NUM_SLOTS_PER_REL; i++) {
        // If the slot is used, load the node and append it to the resulting
        // array list
        if (check_record_exists(hf, i, false)) {
            rel = read_relationship_internal(hf, i, false);
            array_list_relationship_append(result, rel);
        }
    }

    return result;
}

unsigned long
next_relationship_id(heap_file*      hf,
                     unsigned long   node_id,
                     relationship_t* rel,
                     direction_t     direction)
{
    if (!hf || node_id == UNINITIALIZED_LONG || !rel) {
        printf("heap_file - next relationship id: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long start_rel_id = rel->id;
    unsigned long rel_id = node_id == rel->source_node ? rel->next_rel_source
                                                       : rel->next_rel_target;

    do {
        rel = read_relationship(hf, rel_id);

        if (rel_id != start_rel_id
            && ((rel->source_node == node_id && direction != INCOMING)
                || (rel->target_node == node_id && direction != OUTGOING))) {
            return rel->id;
        }
        rel_id = node_id == rel->source_node ? rel->next_rel_source
                                             : rel->next_rel_target;

    } while (rel_id != start_rel_id);

    return UNINITIALIZED_LONG;
}

array_list_relationship*
expand(heap_file* hf, unsigned long node_id, direction_t direction)
{
    if (!hf) {
        printf("heap file - expand: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    node_t* node = read_node_internal(hf, node_id, true);

    array_list_relationship* result = al_rel_create();
    unsigned long            rel_id = node->first_relationship;

    if (rel_id == UNINITIALIZED_LONG) {
        return result;
    }

    relationship_t* rel = read_relationship(hf, rel_id);
    unsigned long   start_id;

    if ((rel->source_node == node_id && direction != INCOMING)
        || (rel->target_node == node_id && direction != OUTGOING)) {
        start_id = rel_id;
    } else {
        rel_id   = next_relationship_id(hf, node_id, rel, direction);
        start_id = rel_id;
    }

    while (rel_id != UNINITIALIZED_LONG) {
        rel = read_relationship(hf, rel_id);
        array_list_relationship_append(result, rel);
        rel_id = next_relationship_id(hf, node->id, rel, direction);

        if (rel_id == start_id) {
            return result;
        }
    }

    return result;
}

relationship_t*
contains_relationship_from_to(heap_file*    hf,
                              unsigned long node_from,
                              unsigned long node_to,
                              direction_t   direction)
{
    if (!hf || node_from == UNINITIALIZED_LONG
        || node_to == UNINITIALIZED_LONG) {
        printf("heap file - contains relationship: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    relationship_t* rel;
    node_t*         source_node = read_node_internal(hf, node_from, true);
    node_t*         target_node = read_node_internal(hf, node_to, true);

    if (source_node->first_relationship == UNINITIALIZED_LONG
        || target_node->first_relationship == UNINITIALIZED_LONG) {
        return NULL;
    }

    unsigned long next_id  = source_node->first_relationship;
    unsigned long start_id = next_id;

    do {
        rel = read_relationship(hf, next_id);
        if ((direction != INCOMING && rel->source_node == node_from
             && rel->target_node == node_to)
            || (direction != OUTGOING && rel->source_node == node_to
                && rel->target_node == node_from)) {
            return rel;
        }
        next_id = next_relationship_id(hf, source_node->id, rel, direction);
    } while (next_id != start_id && next_id != UNINITIALIZED_LONG);

    return NULL;
}
