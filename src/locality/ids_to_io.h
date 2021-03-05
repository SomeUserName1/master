#ifndef IDS_TO_IO
#define IDS_TO_IO

#include <stddef.h>

#define BUFFER_SIZE (128)
#define NUM_CHARS_REL (12)

typedef enum record_id_type {
    NODE = 0,
    REL = 1,
    ALL = 2
} record_id_t;

typedef struct io_stats {
    size_t read_pages;
    size_t read_blocks;
    size_t write_pages;
    size_t write_blocks;
} io_stats_t;

unsigned long id_to_page(unsigned long id, size_t page_size, record_id_t type);

io_stats_t* ids_to_io(const char* in_path, const char* out_path, size_t page_size, size_t block_size, record_id_t type);

#endif
