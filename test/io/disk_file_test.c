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
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    assert(df);
    assert(df->file);
    assert(df->file_size == 0);
    assert(df->num_pages == 0);
    assert(df->read_count == 0);
    assert(df->write_count == 0);
    assert(df->f_buf);
    assert(df->file->_IO_buf_base == df->f_buf);
#ifdef VERBOSE
    assert(df->log_file);
#endif

    if (fclose(df->file) != 0) {
        printf("test_disk_file_creat: error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (remove(df->file_name) != 0) {
        printf("test_disk_file_creat: error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
#ifdef VERBOSE
    if (fclose(df->file) != 0) {
        printf("test_disk_file_creat: error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (remove("test_log") != 0) {
        printf("test_disk_file_creat: error closing file: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    free(df->f_buf);
    free(df);
}

void
test_disk_file_destroy(void)
{
    char* file_name = "test_file";
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    assert(df);

    disk_file_destroy(df);

    if (remove(df->file_name) != 0) {
        printf("test_disk_file_creat: error removing file: %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }

#ifdef VERBOSE
    if (remove("test_log") != 0) {
        printf("test_disk_file_creat: error removing file: %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
}

void
test_disk_file_delete(void)
{
    char* file_name = "test_file";
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    assert(df);

    disk_file_delete(df);

    assert(!fopen("test_file", "r"));

#ifdef VERBOSE
    assert(!fopen("test_log", "r"));
#endif
}

void
test_disk_file_grow(void)
{
    char* file_name = "test_file";
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    assert(df->write_count == 0);
    assert(df->num_pages == 0);
    assert(df->file_size == 0);

    disk_file_grow(df, 1);

    assert(df->write_count == 1);
    assert(df->num_pages == 1);
    assert(df->file_size == PAGE_SIZE);

    unsigned char* data = malloc(PAGE_SIZE * sizeof(unsigned char));
    memset(data, '1', PAGE_SIZE);

    rewind(df->file);
    if (fread(data, PAGE_SIZE, 1, df->file) != PAGE_SIZE) {
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

    disk_file_grow(df, NUM_TEST_PAGES);

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
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    disk_file_grow(df, NUM_TEST_PAGES);

    assert(df->write_count == 1);
    assert(df->num_pages == NUM_TEST_PAGES);
    assert(df->file_size == NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_shrink(df, 2);

    assert(df->write_count == 2);
    assert(df->num_pages == NUM_TEST_PAGES - 2);
    assert(df->file_size == (NUM_TEST_PAGES - 2) * PAGE_SIZE);

    disk_file_delete(df);
}

void
test_write_page(void)
{
    char* file_name = "test_file";
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    unsigned char* data = malloc(PAGE_SIZE * sizeof(unsigned char));

    memset(data, '1', PAGE_SIZE);

    disk_file_grow(df, 1);

    assert(df->write_count == 1);
    assert(df->num_pages == 1);
    assert(df->file_size == PAGE_SIZE);

    write_page(df, 0, data);

    assert(df->write_count == 2);
    assert(df->num_pages == 1);
    assert(df->file_size == PAGE_SIZE);

    memset(data, '0', PAGE_SIZE);

    rewind(df->file);
    if (fread(data, PAGE_SIZE, 1, df->file) != PAGE_SIZE) {
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
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, '1', NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_grow(df, NUM_TEST_PAGES);

    assert(df->write_count == 1);
    assert(df->num_pages == NUM_TEST_PAGES);
    assert(df->file_size == NUM_TEST_PAGES * PAGE_SIZE);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data);

    assert(df->write_count == 2);
    assert(df->num_pages == NUM_TEST_PAGES);
    assert(df->file_size == NUM_TEST_PAGES * PAGE_SIZE);

    memset(data, '0', NUM_TEST_PAGES * PAGE_SIZE);

    rewind(df->file);
    if (fread(data, PAGE_SIZE, NUM_TEST_PAGES, df->file) != PAGE_SIZE) {
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
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    unsigned char* data = malloc(PAGE_SIZE * sizeof(unsigned char));

    memset(data, '1', PAGE_SIZE);

    disk_file_grow(df, 1);

    write_page(df, 0, data);

    memset(data, '0', PAGE_SIZE);

    read_page(df, 0, data);

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
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, '1', NUM_TEST_PAGES * PAGE_SIZE);

    disk_file_grow(df, NUM_TEST_PAGES);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data);

    memset(data, '0', PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data);

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
#ifdef VERBOSE
    FILE* log_file = fopen("test_log", "a+");
    if (!log_file) {
        printf("test disk file: failed to open test log file %s",
               strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    disk_file* df = disk_file_create(file_name
#ifdef VERBOSE
                                     ,
                                     log_file
#endif
    );

    unsigned char* data =
          malloc(NUM_TEST_PAGES * PAGE_SIZE * sizeof(unsigned char));

    memset(data, '1', NUM_TEST_PAGES * PAGE_SIZE);

    write_pages(df, 0, NUM_TEST_PAGES - 1, data);

    memset(data, '0', PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data);

    assert(df->read_count == 1);

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE; ++i) {
        assert(data[i] == 1);
    }

    clear_page(df, 1);

    memset(data, '9', PAGE_SIZE);

    read_pages(df, 0, NUM_TEST_PAGES - 1, data);

    clear_page(df, 1);

    for (size_t i = 0; i < NUM_TEST_PAGES * PAGE_SIZE; ++i) {
        if (i / PAGE_SIZE == 1) {
            assert(data[i] == 0);
        } else {
            assert(data[i] == 1);
        }
    }

    assert(df->write_count == 2);

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
