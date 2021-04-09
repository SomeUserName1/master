#ifndef IDS_TO_IO
#define IDS_TO_IO

#include <stddef.h>

#define BUFFER_SIZE (512)

typedef enum record_id_type
{
    NODE = 0,
    REL  = 1,
    ALL  = 2
} record_id_t;

void
ids_to_blocks(const char* in_path, const char* out_path, record_id_t type);

void
blocks_to_pages(const char* in_path, const char* out_path, record_id_t type);

#endif
