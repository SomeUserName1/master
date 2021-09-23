/*
 * @(#)disk_file.h   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef DISK_FILE_H
#define DISK_FILE_H

#include <stdbool.h>
#include <stdio.h>

typedef struct
{
    char*  file_name;
    FILE*  file;
    long   file_size;
    size_t num_pages;
    size_t read_count;
    size_t write_count;
    char*  f_buf;
    FILE*  log_file;
} disk_file;

disk_file*
disk_file_create(char* file_name, FILE* log_file);

disk_file*
disk_file_open(char* file_name, FILE* log_file);

void
disk_file_destroy(disk_file* df);

void
disk_file_delete(disk_file* df);

void
read_page(disk_file* df, size_t page_no, unsigned char* buf, bool log);

void
read_pages(disk_file*     df,
           size_t         fst_page,
           size_t         lst_page,
           unsigned char* buf,
           bool           log);

void
write_page(disk_file* df, size_t page_no, unsigned char* data, bool log);

void
write_pages(disk_file*     df,
            size_t         fst_page,
            size_t         lst_page,
            unsigned char* data,
            bool           log);

void
disk_file_sync(disk_file* df);

void
clear_page(disk_file* df, size_t page_no, bool log);

void
disk_file_grow(disk_file* df, size_t by_num_pages, bool log);

void
disk_file_shrink(disk_file* df, size_t by_num_pages, bool log);

#endif
