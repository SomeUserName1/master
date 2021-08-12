#include "disk_file.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"

#define PiB_OFFSET 50

disk_file*
disk_file_create(char* file_name
#ifdef VERBOSE
                 ,
                 FILE* log_file
#endif
)
{
    if (!file_name) {
        printf("disk file - create: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    FILE* new_file = fopen(file_name, "wb");

    if (!new_file) {
        printf("disk file - create: Failed to create file %s: %s!\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fclose(new_file) != 0) {
        printf(
              "disk file - create: Failed to close newly created file %s: %s\n",
              file_name,
              strerror(errno));
        exit(EXIT_FAILURE);
    }

    return disk_file_open(file_name
#ifdef VERBOSE
                          ,
                          log_file
#endif
    );
}

disk_file*
disk_file_open(char* file_name
#ifdef VERBOSE
                     FILE* log_file
#endif
)
{
    if (!file_name) {
        printf("disk file - create: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    disk_file* df = calloc(1, sizeof(disk_file));

    if (!df) {
        printf("disk file - create: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    df->file_name = file_name;
    df->file      = fopen(file_name, "rb+");

    if (df->file == NULL) {
        printf("disk file - create: failed to fopen %s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    df->f_buf = calloc(PAGE_SIZE << 3, sizeof(char));

    if (!df->f_buf) {
        printf("disk file - create: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    if (setvbuf(df->file, df->f_buf, _IOFBF, PAGE_SIZE << 3) != 0) {
        printf("disk file - create: failed to set the internal buffer of file "
               "%s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fseek(df->file, 0, SEEK_END) == -1) {
        printf("disk file - create: failed to fseek %s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->file_size = ftell(df->file);

    if (df->file_size == -1) {
        printf("disk file - create: failed to ftell %s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->num_pages   = df->file_size / PAGE_SIZE;
    df->read_count  = 0;
    df->write_count = 0;

#ifdef VERBOSE
    if (!log_file) {
        printf("disk file - create: No log file was provided!\n");
        exit(EXIT_FAILURE);
    }

    df->log_file = log_file;
#endif

    return df;
}

void
disk_file_destroy(disk_file* df)
{
    if (!df) {
        printf("disk file - destroy: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fclose(df->file) != 0) {
        printf("disk file - destroy: Error closing file: %s", strerror(errno));
    }

    free(df->f_buf);

    free(df);
}

void
disk_file_delete(disk_file* df)
{
    if (!df) {
        printf("disk file - delete: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fclose(df->file) != 0) {
        printf("disk file - delete: Error closing file: %s", strerror(errno));
    }

    if (remove(df->file_name) != 0) {
        printf("disk file - delete: Error removing file %s: %s\n",
               df->file_name,
               strerror(errno));
    } else {
    }

    free(df->f_buf);
    free(df);
}

void
disk_file_grow(disk_file* df, size_t by_num_pages)
{
    if (!df) {
        printf("disk file - grow: invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (df->num_pages + by_num_pages >= MAX_PAGE_NO) {
        printf("disk file - grow: Cannot grow database by %lu pages! "
               "Exceeds "
               "max database size "
               "of %li PiB!\n",
               by_num_pages,
               LONG_MAX >> PiB_OFFSET);
        exit(EXIT_FAILURE);
    }

    if (fseek(df->file, 0, SEEK_END) == -1) {
        printf("disk file - grow: failed to fseek with errno %d\n", errno);
        exit(EXIT_FAILURE);
    }

    char* data = calloc(by_num_pages, PAGE_SIZE);

    if (!data) {
        printf("disk page - grow: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    size_t res;
    if ((res = fwrite(data, PAGE_SIZE, by_num_pages, df->file)
               != by_num_pages)) {
        printf("disk file - grow pages: Failed to grow file %s wrote %lu "
               "objects instead of %lu: %s\n",
               df->file_name,
               res,
               by_num_pages,
               strerror(errno));
        exit(EXIT_FAILURE);
    } else {
#ifdef VERBOSE
        fprintf(df->log_file, "grow_file %s %lu", df->file_name, by_num_pages);
#endif
    }

    free(data);

    df->file_size = ftell(df->file);

    if (df->file_size == -1) {
        printf("disk file - grow: failed to ftell file %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->num_pages = df->file_size / PAGE_SIZE;

    df->write_count++;
}

/**
 * DANGER ZONE!
 * This method assumes that the empty pages have been moved to the end.
 * It will simply shrink the file by cutting of the last by_no_pages pages.
 * If these are not empty, the records on these pages will be lost!
 */
void
disk_file_shrink(disk_file* df, size_t by_num_pages)
{
    if (!df || by_num_pages > MAX_PAGE_NO) {
        printf("disk file - shrink: invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (df->num_pages - by_num_pages <= 0) {
        printf("disk file - shrink: Cannot shrink database by %lu pages! "
               "The "
               "database would have "
               "less than 0 pages!\n",
               by_num_pages);
        exit(EXIT_FAILURE);
    }

    if (fflush(df->file) != 0) {
        printf("disk file - flush: flushing failed: %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int fd = fileno(df->file);

    if (fd == -1) {
        printf("disk file - shrink: Failed to get file descriptor from "
               "stream "
               "of file %s: %s\n",
               df->file_name,
               strerror(errno));
    }

    long shrink_by_bytes = PAGE_SIZE * by_num_pages;
    if (ftruncate(fd, df->file_size - shrink_by_bytes) != 0) {
        printf("disk file - shrink: Failed to truncate the file %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    } else {
#ifdef VERBOSE
        fprintf(
              df->log_file, "shrink_file %s %lu", df->file_name, by_num_pages);
#endif
    }

    if (fseek(df->file, 0, SEEK_END) == -1) {
        printf("disk file - shrink: failed to fseek with errno %d\n", errno);
        exit(EXIT_FAILURE);
    }

    df->file_size = ftell(df->file);

    if (df->file_size == -1) {
        printf("disk file - shrink: failed to ftell file %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->num_pages = df->file_size / PAGE_SIZE;

    df->write_count++;
}

void
write_page(disk_file* df, size_t page_no, unsigned char* data)
{
    write_pages(df, page_no, page_no, data);
}

void
write_pages(disk_file*     df,
            size_t         fst_page,
            size_t         lst_page,
            unsigned char* data)
{
    if (!df || !data) {
        printf("disk file - write page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fst_page > MAX_PAGE_NO || lst_page > MAX_PAGE_NO) {
        printf("disk file - write page: One of the page numbers you "
               "specified "
               "(%lu or %lu) are "
               "too large!\n "
               "The size limit of the database is currently %li PiB, that "
               "is "
               "the maximal page number is %li",
               fst_page,
               lst_page,
               LONG_MAX >> PiB_OFFSET,
               MAX_PAGE_NO);
        exit(EXIT_FAILURE);
    }

    if (fst_page > lst_page) {
        printf("disk file - write pages: The number of the last page to be "
               "read "
               "needs to be larger than the number of the first page to be "
               "read!\n");
        exit(EXIT_FAILURE);
    }

    if (lst_page + 1 > df->num_pages) {
        // +1 as the page indexes start at 0 while the number of pages
        // starts counting at 1
        printf("disk file - write: File not large enough! You need to "
               "allocate "
               "more pages using the respective function of the physical "
               "database.\n");
        exit(EXIT_FAILURE);
    }

    long offset = (long)(PAGE_SIZE * fst_page);

    size_t num_pages_write = 1 + (lst_page - fst_page);

    if (fseek(df->file, offset, SEEK_SET) == -1) {
        printf("disk file - write page: failed to fseek %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fwrite(data, PAGE_SIZE, num_pages_write, df->file) != num_pages_write) {
        printf("disk file - write pages: Failed to write the pages from "
               "%lu to "
               "%lu from file %s: %s\n",
               fst_page,
               lst_page,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    } else {
#ifdef VERBOSE
        fprintf(df->log_file,
                "write_pages %s %lu %lu",
                df->file_name,
                fst_page,
                lst_page);
#endif
    }

    df->write_count++;
}

void
read_page(disk_file* df, size_t page_no, unsigned char* buf)
{
    read_pages(df, page_no, page_no, buf);
}

void
read_pages(disk_file* df, size_t fst_page, size_t lst_page, unsigned char* buf)
{
    if (!df || !buf) {
        printf("disk file - read pages: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fst_page > MAX_PAGE_NO || lst_page > MAX_PAGE_NO
        || fst_page > df->num_pages || lst_page > df->num_pages) {
        printf("disk file - read pages: One of the page numbers you "
               "specified "
               "(%lu or %lu) are "
               "too large!\n "
               "The current size of the database is %lu pages and %li "
               "bytes.\n"
               "The size limit of the database is currently %li PiB, that "
               "is "
               "the maximal page number is %li",
               fst_page,
               lst_page,
               df->num_pages,
               df->file_size,
               LONG_MAX >> PiB_OFFSET,
               MAX_PAGE_NO);
        exit(EXIT_FAILURE);
    }

    if (fst_page > lst_page) {
        printf("disk file - read pages: The number of the last page to be "
               "read "
               "needs to be larger than the number of the first page to be "
               "read!\n");
        exit(EXIT_FAILURE);
    }

    long offset = (long)(PAGE_SIZE * fst_page);

    size_t num_pages_read = 1 + (lst_page - fst_page);

    if (fseek(df->file, offset, SEEK_SET) == -1) {
        printf("disk file - read pages: failed to fseek %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fread(buf, PAGE_SIZE, num_pages_read, df->file) != num_pages_read) {
        printf("disk file - read pages: Failed to read the pages from %lu "
               "to "
               "%lu from file %s: %s\n",
               fst_page,
               lst_page,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    } else {
#ifdef VERBOSE
        fprintf(df->log_file,
                "read_page %s %lu %lu",
                df->file_name,
                fst_page,
                lst_page);
#endif
    }

    df->read_count++;
}

void
clear_page(disk_file* df, size_t page_no)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    if (!data) {
        printf("disk page - clear page: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    write_page(df, page_no, data);
    free(data);
}

