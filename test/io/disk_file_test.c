/*
 * @(#)disk_file_test.c   1.0   Sep 15, 2021
 *
 * Copyright (c) 2021- University of Konstanz.
 *
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "disk_file.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

#define NUM_TEST_PAGES (10)

void
test_disk_file_create(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    assert(df);
    assert(df->file);
    assert(df->file_size == 0);
    assert(df->num_pages == 0);
    assert(df->read_count == 0);
    assert(df->write_count == 0);
    assert(df->f_buf);
    assert(df->file->_IO_buf_base == df->f_buf);
    assert(df->log_file);

    if (fclose(df->file) != 0) {
        printf("test_disk_file_creat: error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    free(df->f_buf);
    free(df);
}

void
test_disk_file_destroy(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    assert(df);

    disk_file_destroy(df);
}

void
test_disk_file_open(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    assert(df);

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, 1, NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_grow(df, NUM_TEST_PAGES, false);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    disk_file_destroy(df);

    df = disk_file_open(file_name, false);

    memset(data, 0, NUM_TEST_PAGES * PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE - 1; ++i) {
        assert(data[i] == 1);
    }

    disk_file_delete(df);
}

void
test_disk_file_delete(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    assert(df);

    disk_file_delete(df);

    assert(!fopen("test_file", "r"));
}

void
test_disk_file_grow(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    assert(df->write_count == 0);
    assert(df->num_pages == 0);
    assert(df->file_size == 0);

    disk_file_grow(df, 1, true);

    assert(df->write_count == 1);
    assert(df->num_pages == 1);
    assert(df->file_size == PAGE_SIZE);

    unsigned char* data = malloc(PAGE_SIZE * sizeof(unsigned char));
    memset(data, '1', PAGE_SIZE);

    rewind(df->file);
    if (fread(data, PAGE_SIZE, 1, df->file) != 1) {
        printf("test write disk file: Failed to read the page %lu from file "
               "%s: %s\n",
               0L,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < PAGE_SIZE; ++i) {
        assert(!data[i]);
    }

    disk_file_grow(df, NUM_TEST_PAGES, false);

    assert(df->write_count == 2);
    assert(df->num_pages == NUM_TEST_PAGES + 1);
    assert(df->file_size == (NUM_TEST_PAGES + 1) * PAGE_SIZE);

    disk_file_delete(df);
    free(data);
}

void
test_disk_file_shrink(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    disk_file_grow(df, NUM_TEST_PAGES, false);

    assert(df->write_count == 1);
    assert(df->num_pages == NUM_TEST_PAGES);
    assert(df->file_size == NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_shrink(df, 2, true);

    assert(df->write_count == 2);
    assert(df->num_pages == NUM_TEST_PAGES - 2);
    assert(df->file_size == (NUM_TEST_PAGES - 2) * PAGE_SIZE);

    assert(fseek(df->file, 0, SEEK_END) == 0);
    assert(ftell(df->file) == (NUM_TEST_PAGES - 2) * PAGE_SIZE);

    disk_file_delete(df);
}

void
test_write_page(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    unsigned char* data = malloc(PAGE_SIZE * sizeof(unsigned char));

    memset(data, 1, PAGE_SIZE);

    disk_file_grow(df, 1, false);

    assert(df->write_count == 1);
    assert(df->num_pages == 1);
    assert(df->file_size == PAGE_SIZE);

    write_page(df, 0, data, true);

    assert(df->write_count == 2);
    assert(df->num_pages == 1);
    assert(df->file_size == PAGE_SIZE);

    memset(data, 0, PAGE_SIZE);

    rewind(df->file);

    if (fread(data, PAGE_SIZE, 1, df->file) != 1) {
        printf("test write disk file: Failed to read the page %lu from file "
               "%s: %s\n",
               0L,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    disk_file_delete(df);
    free(data);
}

void
test_write_pages(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, 1, NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_grow(df, NUM_TEST_PAGES, false);

    assert(df->write_count == 1);
    assert(df->num_pages == NUM_TEST_PAGES);
    assert(df->file_size == NUM_TEST_PAGES * PAGE_SIZE);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    assert(df->write_count == 2);
    assert(df->num_pages == NUM_TEST_PAGES);
    assert(df->file_size == NUM_TEST_PAGES * PAGE_SIZE);

    memset(data, 0, NUM_TEST_PAGES * PAGE_SIZE);

    rewind(df->file);
    if (fread(data, PAGE_SIZE, NUM_TEST_PAGES, df->file) != NUM_TEST_PAGES) {
        printf("test write disk file: Failed to read the page %lu from file "
               "%s: %s\n",
               0L,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    disk_file_delete(df);
    free(data);
}

void
test_read_page(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    unsigned char* data = malloc(PAGE_SIZE * sizeof(unsigned char));

    memset(data, 1, PAGE_SIZE);

    disk_file_grow(df, 1, false);

    write_page(df, 0, data, false);

    memset(data, 0, PAGE_SIZE);

    read_page(df, 0, data, true);

    assert(df->read_count == 1);

    for (size_t i = 0; i < PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    disk_file_delete(df);
    free(data);
}

void
test_read_pages(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, 1, NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_grow(df, NUM_TEST_PAGES, false);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    memset(data, 0, PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    assert(df->read_count == 1);

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    disk_file_delete(df);
    free(data);
}

void
test_clear_page(void)
{
    char* file_name = "test_file";
    FILE* log_file  = fopen("test_log", "a");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    disk_file* df = disk_file_create(file_name, log_file);

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, 1, NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_grow(df, NUM_TEST_PAGES, false);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    memset(data, 0, PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    assert(df->read_count == 1);

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    clear_page(df, 1, true);

    memset(data, 3, PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data, false);

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE; ++i) {
        if (i / PAGE_SIZE == 1) {
            assert(data[i] == 0);
        } else {
            assert(data[i] == 1);
        }
    }

    assert(df->write_count == 3);

    disk_file_delete(df);
    free(data);
}

int
main(void)
{
    test_disk_file_create();
    test_disk_file_destroy();
    test_disk_file_delete();
    test_disk_file_grow();
    test_disk_file_shrink();
    test_write_page();
    test_write_pages();
    test_read_page();
    test_read_pages();
    test_clear_page();

    return 0;
}
