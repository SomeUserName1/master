#include "page.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

#define TEST_BYTE               (1 << 5) // is 0010 0000
#define TEST_NUMBER             (42)
#define DOUBLE_MINUS_TWO_PREFIX (0xC0)
#define DOUBLE_42_FST_BYTE      (0x40)
#define DOUBLE_42_SND_BYTE      (0x45)
static const double minus_two   = -2.0;
static const double TEST_DOUBLE = 123.456;
static const double D_MIN       = DBL_MIN;
static const double D_MAX =
      DBL_MAX; // Compiler compains about magic number when simply using DBL_MAX
               // in read/write double tests

void
test_page_create(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    assert(p);
    assert(p->data == data);
    assert(p->page_no == ULONG_MAX);
    assert(p->pin_count == 0);
    assert(p->dirty == false);

    free(data);
    free(p);

    printf("page test - create: Successful!\n");
}

void
test_page_destroy(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    page_destroy(p);
    free(data);
    // Checks memory leaks

    printf("page test - destroy: Successful!\n");
}

void
test_read_ulong(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    memset(data, UCHAR_MAX, sizeof(unsigned long));
    memset(data + sizeof(unsigned long), TEST_BYTE, sizeof(unsigned long));

    unsigned long check_first  = read_ulong(p, 0);
    unsigned long check_second = read_ulong(p, sizeof(unsigned long));

    unsigned long result = 0;
    for (size_t i = 0; i < sizeof(unsigned long); ++i) {
        result = (result << CHAR_BIT) | TEST_BYTE;
    }

    assert(check_first == ULONG_MAX);
    assert(check_second == result);

    page_destroy(p);
    free(data);
    printf("page test - read ulong: Successful!\n");
}

void
test_write_ulong(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    write_ulong(p, 0, TEST_NUMBER);
    write_ulong(p, sizeof(unsigned long), TEST_BYTE);
    write_ulong(p, PAGE_SIZE - sizeof(unsigned long), ULONG_MAX);

    unsigned long check_first  = read_ulong(p, 0);
    unsigned long check_second = read_ulong(p, sizeof(unsigned long));
    unsigned long check_third =
          read_ulong(p, PAGE_SIZE - sizeof(unsigned long));

    assert(check_first == TEST_NUMBER);
    assert(check_second == TEST_BYTE);
    assert(check_third == ULONG_MAX);

    p->dirty = false;
    page_destroy(p);
    free(data);
    printf("page test - write ulong: Successful!\n");
}

void
test_read_uchar(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    memset(data, UCHAR_MAX, sizeof(unsigned char));
    memset(data + sizeof(unsigned char), TEST_BYTE, sizeof(unsigned char));

    unsigned char check_first  = read_uchar(p, 0);
    unsigned char check_second = read_uchar(p, sizeof(unsigned char));

    assert(check_first == UCHAR_MAX);
    assert(check_second == TEST_BYTE);

    page_destroy(p);
    free(data);
    printf("page test - read uchar: Successful!\n");
}

void
test_write_uchar(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    write_uchar(p, 0, TEST_NUMBER);
    write_uchar(p, sizeof(unsigned char), TEST_BYTE);
    write_uchar(p, PAGE_SIZE - sizeof(unsigned char), UCHAR_MAX);

    unsigned char check_first  = read_uchar(p, 0);
    unsigned char check_second = read_uchar(p, sizeof(unsigned char));
    unsigned char check_third =
          read_uchar(p, PAGE_SIZE - sizeof(unsigned char));

    assert(check_first == TEST_NUMBER);
    assert(check_second == TEST_BYTE);
    assert(check_third == UCHAR_MAX);

    p->dirty = false;
    page_destroy(p);
    free(data);
    printf("page test - write uchar: Successful!\n");
}

void
test_read_double(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    memset(data, 0, sizeof(double));
    data[sizeof(double) - 1] = DOUBLE_MINUS_TWO_PREFIX;
    memset(data + sizeof(double), 0, sizeof(double));
    data[2 * sizeof(double) - 2] = DOUBLE_42_SND_BYTE;
    data[2 * sizeof(double) - 1] = DOUBLE_42_FST_BYTE;

    double check_first  = read_double(p, 0);
    double check_second = read_double(p, sizeof(double));

    assert(check_first == minus_two);
    assert(check_second == (double)TEST_NUMBER);

    page_destroy(p);
    free(data);
    printf("page test - read double: Successful!\n");
}

void
test_write_double(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    write_double(p, 0, TEST_DOUBLE);
    write_double(p, sizeof(double), TEST_DOUBLE + 1);
    write_double(p, PAGE_SIZE - (2 * sizeof(double)), D_MIN);
    write_double(p, PAGE_SIZE - sizeof(double), D_MAX);

    double check_first  = read_double(p, 0);
    double check_second = read_double(p, sizeof(double));
    double check_third  = read_double(p, PAGE_SIZE - (2 * sizeof(double)));
    double check_fourth = read_double(p, PAGE_SIZE - sizeof(double));

    assert(check_first == TEST_DOUBLE);
    assert(check_second == TEST_DOUBLE + 1);
    assert(check_third == D_MIN);
    assert(check_fourth == D_MAX);

    p->dirty = false;
    page_destroy(p);
    free(data);
    printf("page test - write double: Successful!\n");
}

void
test_read_string(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    memset(data, 'a', MAX_STR_LEN - 1);
    data[MAX_STR_LEN - 1] = '\0';
    memset(data + MAX_STR_LEN, 'b', MAX_STR_LEN - 1);
    data[2 * MAX_STR_LEN - 1] = '\0';

    char check_first[MAX_STR_LEN];
    read_string(p, 0, check_first);
    char check_second[MAX_STR_LEN];
    read_string(p, MAX_STR_LEN, check_second);

    for (size_t i = 0; i < MAX_STR_LEN - 1; ++i) {
        assert(check_first[i] == 'a');
        assert(check_second[i] == 'b');
    }
    assert(check_first[MAX_STR_LEN - 1] == '\0');
    assert(check_second[MAX_STR_LEN - 1] == '\0');

    page_destroy(p);
    free(data);
    printf("page test - read string: Successful!\n");
}

void
test_write_string(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    char write_first[MAX_STR_LEN];
    char write_snd[MAX_STR_LEN];

    memset(write_first, 'a', MAX_STR_LEN - 1);
    write_first[MAX_STR_LEN - 1] = '\0';
    memset(write_snd, 'b', MAX_STR_LEN - 1);
    write_snd[MAX_STR_LEN - 1] = '\0';

    write_string(p, 0, write_first);
    write_string(p, MAX_STR_LEN, write_snd);

    write_string(p, PAGE_SIZE - MAX_STR_LEN, write_first);

    char check_first[MAX_STR_LEN];
    read_string(p, 0, check_first);
    char check_second[MAX_STR_LEN];
    read_string(p, MAX_STR_LEN, check_second);
    char check_third[MAX_STR_LEN];
    read_string(p, PAGE_SIZE - MAX_STR_LEN, check_third);

    for (size_t i = 0; i < MAX_STR_LEN - 1; ++i) {
        assert(check_first[i] == 'a');
        assert(check_second[i] == 'b');
        assert(check_third[i] == 'a');
    }

    assert(check_first[MAX_STR_LEN - 1] == '\0');
    assert(check_second[MAX_STR_LEN - 1] == '\0');
    assert(check_third[MAX_STR_LEN - 1] == '\0');

    p->dirty = false;
    page_destroy(p);
    free(data);
    printf("page test - write string: Successful!\n");
}

void
test_page_pretty_print(void)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    page* p = page_create(data);

    page_pretty_print(p);

    page_destroy(p);
    free(data);
    printf("page test - pretty print: Successful!\n");
}

int
main(void)
{
    test_page_create();
    test_page_destroy();
    test_read_ulong();
    test_write_ulong();
    test_read_uchar();
    test_write_uchar();
    test_read_double();
    test_write_double();
    test_read_string();
    test_write_string();
    test_page_pretty_print();

    return 0;
}
