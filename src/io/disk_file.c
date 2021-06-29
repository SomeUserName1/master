#include "disk_file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"

disk_file*
disk_file_create(const char* file_name)
{
    if (!file_name) {
        printf("disk file - create: Invalid arguments!\n");
        exit(-1);
    }

    disk_file* df = calloc(1, sizeof(disk_file));

    if (!df) {
        printf("disk file - create: Failed to allocate memory!\n");
        exit(-1);
    }

    df->file_name = file_name;
    df->file      = fopen(file_name, "ab+");

    if (df->file == NULL) {
        printf("disk file - failed to fopen %s with errno %d\n",
               file_name,
               errno);
        exit(-1);
    }

    df->f_buf = calloc(PAGE_SIZE << 3, sizeof(char));
    setbuf(df->file, df->f_buf);

    if (fseek(df->file, 0, SEEK_END) == -1) {
        printf("failed to fseek %s with errno %d\n", file_name, errno);
        exit(-1);
    }

    df->file_size = ftell(df->file);

    if (df->file_size == -1) {
        printf("failed to ftell %s, with errno %d\n", file_name, errno);
        exit(EXIT_FAILURE);
    }

    df->num_pages = df->file_size / PAGE_SIZE;

    return df;
}

void
disk_file_destroy(disk_file* df)
{
    fclose(df->file);
    free(df->f_buf);
    free(df);
}

void
read_page(disk_file* df, size_t page_no, char* buf)
{
    fsetpos(df->file, page_no * PAGE_SIZE); // TODO use fpos_t
    fread(buf, PAGE_SIZE, 1, df->file);
}

unsigned char*
read_pages(disk_file* df, size_t fst_page, size_t lst_page)
{}

void
write_page(disk_file* df, size_t page_no, unsigned char* data)
{}

void
write_pages(disk_file*      df,
            size_t          fst_page,
            size_t          last_page,
            unsigned char** data)
{}

void
clear_page(disk_file* df, size_t page_no)
{}

void
disk_file_grow(disk_file* df, size_t by_num_pages)
{}

void
disk_file_shrink(disk_file* df, size_t by_no_pages)
{}

