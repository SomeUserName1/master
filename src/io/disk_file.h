#ifndef DISK_FILE_H
#define DISK_FILE_H

#include <stddef.h>
#include <stdio.h>

typedef struct
{
    char*  file_name;
    FILE*  file;
    size_t num_pages;
} disk_file;

disk_file*
disk_file_create(char* file_name);

void
disk_file_destroy(disk_file* df);

size_t
size(disk_file* df);

size_t
get_free_page(disk_file* df);

size_t
get_num_pages(disk_file* df);

unsigned char*
read_page(disk_file* df, size_t page_no);

unsigned char*
read_pages(disk_file* df, size_t fst_page, size_t lst_page);

void
write_page(disk_file* df, size_t page_no, unsigned char* data);

void
write_pages(disk_file*      df,
            size_t          fst_page,
            size_t          last_page,
            unsigned char** data);

void
clear_page(disk_file* df, size_t page_no);

void
disk_file_grow(disk_file* df, size_t by_num_pages);

void
disk_file_shrink(disk_file* df, size_t by_no_pages);

#endif
