#ifndef DISK_FILE_H
#define DISK_FILE_H

#include <stdio.h>

typedef struct
{
    char*  file_name;
    FILE*  file;
    long   file_size;
    size_t num_pages;
    char*  f_buf;
} disk_file;

disk_file*
disk_file_create(char* file_name);

void
disk_file_destroy(disk_file* df);

void
disk_file_delete(disk_file* df);

void
read_page(disk_file* df, size_t page_no, char* buf);

void
read_pages(disk_file* df, size_t fst_page, size_t lst_page, char* buf);

void
write_page(disk_file* df, size_t page_no, char* data);

void
write_pages(disk_file* df, size_t fst_page, size_t lst_page, char* data);

void
clear_page(disk_file* df, size_t page_no);

void
disk_file_grow(disk_file* df, size_t by_num_pages);

void
disk_file_shrink(disk_file* df, size_t by_num_pages);

#endif
