#include "layout/random_layout.h"

#include <stdio.h>
#include <stdlib.h>

unsigned long*
identity_partition(in_memory_file_t* db)
{
    if (!db) {
        printf("trivial partitions - identity: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long* partition =
          calloc(db->node_id_counter, sizeof(unsigned long));

    if (!partition) {
        printf("trivial partitions - identity: Memory Allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        partition[i] = i;
    }

    return partition;
}

unsigned long*
random_partition(in_memory_file_t* db)
{
    if (!db) {
        printf("trivial partitions - identity: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    unsigned long* partition =
          calloc(db->node_id_counter, sizeof(unsigned long));

    if (!partition) {
        printf("trivial partitions - identity: Memory Allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < db->node_id_counter; ++i) {
        partition[i] = i;
    }
    size_t        pos;
    unsigned long temp;
    for (size_t i = 0; i < db->node_id_counter; ++i) {
        pos            = rand() % (db->node_id_counter);
        temp           = partition[i];
        partition[i]   = partition[pos];
        partition[pos] = temp;
    }

    return partition;
}
