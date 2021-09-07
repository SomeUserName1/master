#include "access/header_page.h"
#include "constants.h"
#include "disk_file.h"
#include "page.h"
#include "page_cache.h"
#include "physical_database.h"

#include <assert.h>
#include <limits.h>

static const unsigned char test_number = 5;
static const unsigned long num_pages_for_two_header_p =
      1 + PAGE_SIZE * CHAR_BIT / SLOTS_PER_PAGE;
static const unsigned char test_case_shift = 7;
static const unsigned char test_case_write = 6;

void
test_compare_bits(void)
{
    const unsigned char test[1]   = { test_number };
    const unsigned char test_2[2] = { test_number, UCHAR_MAX };

    assert(compare_bits(test, 1 * CHAR_BIT, 1, 0, 6));
    assert(!compare_bits(test, 1 * CHAR_BIT, 1 << 4, 0, 5));
    assert(compare_bits(test, 1 * CHAR_BIT, test_number, 0, 8));

    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 8, 4));
    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 8, 8));
    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 7, 8));
    assert(!compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 4, 8));

    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX >> 4, 8, 4));
    assert(!compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX >> 4, 8, 5));
    assert(!compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX >> 4, 7, 5));
    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 8, 4));
    assert(!compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 4, 4));
    assert(compare_bits(test_2, 2 * CHAR_BIT, test_number, 4, 4));

    printf("test header page - compare bits successful\n");
}

void
test_shift_bit_array(void)
{
    unsigned char test[2] = { test_number, UCHAR_MAX };

    shift_bit_array(test, 2 * CHAR_BIT, 3);

    assert(test[0] == 0);
    assert(test[1] == UCHAR_MAX - (1 << 6));

    unsigned char test_2[2] = { test_number, UCHAR_MAX };

    shift_bit_array(test_2, 2 * CHAR_BIT, -test_number);

    assert(test_2[0] == UCHAR_MAX - (1 << 6));
    assert(test_2[1] == (unsigned char)(UCHAR_MAX << 5));

    unsigned char test_3[2] = { 0, test_case_shift };

    shift_bit_array(test_3, 2 * CHAR_BIT, -((char)test_case_shift));
    assert(test_3[0] == 3);
    assert(test_3[1] == 128);

    printf("test header page - shift bit array successful\n");
}

void
test_concat_bit_arrays(void)
{
    unsigned char* test = calloc(2, sizeof(unsigned char));
    test[0]             = test_number;
    test[1]             = 1;

    unsigned char* test_1 = calloc(1, sizeof(unsigned char));
    test_1[0]             = UCHAR_MAX;

    unsigned char* result =
          concat_bit_arrays(test, test_1, 2 * CHAR_BIT, 1 * CHAR_BIT);

    assert(result[0] == test_number);
    assert(result[1] == 1);
    assert(result[2] == UCHAR_MAX);

    free(result);

    unsigned char* test_2 = calloc(2, sizeof(unsigned char));
    test_2[0]             = test_number;
    test_2[1]             = UCHAR_MAX;

    unsigned char* test_3 = calloc(1, sizeof(unsigned char));
    test_3[0]             = 1;

    unsigned char* result_0 = concat_bit_arrays(
          test_2, test_3, CHAR_BIT + CHAR_BIT / 2, CHAR_BIT / 2);

    assert(result_0[0] == test_number);
    assert(result_0[1] == 241);

    free(result_0);

    printf("test header page - concat bit array successful\n");
}
void
test_split_bit_array(void)
{
    unsigned char* ar = calloc(3, sizeof(unsigned char));
    ar[0]             = UCHAR_MAX;
    ar[1]             = 1;
    ar[2]             = UCHAR_MAX;

    unsigned char** result =
          split_bit_array(ar, 3 * CHAR_BIT, 3 * CHAR_BIT / 2);

    assert(result[0][0] == (unsigned char)(UCHAR_MAX >> 4));
    assert(result[0][1] == (unsigned char)(UCHAR_MAX << 4));
    assert(result[1][0] == 1);
    assert(result[1][1] == UCHAR_MAX);

    free(result[0]);
    free(result[1]);
    free(result);

    unsigned char* data_1 = calloc(2, sizeof(unsigned char));
    data_1[0]             = UCHAR_MAX;
    data_1[1]             = test_number;

    result = split_bit_array(data_1, 2 * CHAR_BIT, CHAR_BIT);

    assert(result[0][0] == UCHAR_MAX);
    assert(result[1][0] == test_number);

    free(result[0]);
    free(result[1]);
    free(result);

    printf("test header page - split bit array successful\n");
}

void
test_read_bits(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, num_pages_for_two_header_p);

    page* p = pin_page(pc, 0, header, node_ft);
    write_uchar(p, 0, 1);

    unsigned char* bits = read_bits(pc, p, 0, CHAR_BIT - 1, 1);

    assert(bits[0] == 1);
    free(bits);

    write_uchar(p, 1, test_number);
    bits = read_bits(pc, p, 0, CHAR_BIT - 1, CHAR_BIT + 1);
    assert(bits[0] == 1);
    assert(bits[1] == test_number);
    free(bits);

    write_uchar(p, PAGE_SIZE - 1, UCHAR_MAX);
    page* next = pin_page(pc, 1, header, node_ft);
    write_uchar(next, 0, test_number);
    bits = read_bits(pc, p, PAGE_SIZE - 1, 2, 2 * CHAR_BIT - 2);

    assert(bits[0] == 63);
    assert(bits[1] == 5);
    free(bits);

    bits = read_bits(pc, p, 0, 1, CHAR_BIT);

    assert(bits[0] == 2);

    free(bits);

    unpin_page(pc, 0, header, node_ft);
    unpin_page(pc, 1, header, node_ft);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("test header page - read bits successful\n");
}

void
test_write_bits(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_ft, num_pages_for_two_header_p);
    clear_page(pdb->header[node_ft], 0);

    page* p = pin_page(pc, 0, header, node_ft);

    unsigned char* data = malloc(sizeof(unsigned char));
    data[0]             = 1;
    write_bits(pc, p, 1, 1, 1, data);

    assert(p->data[1] == (1 << (CHAR_BIT - 2)));
    unsigned char* data_1 = calloc(2, sizeof(unsigned char));
    data_1[0]             = UCHAR_MAX;
    data_1[1]             = test_number;
    write_bits(
          pc, p, PAGE_SIZE - 1, CHAR_BIT / 2, CHAR_BIT + CHAR_BIT / 2, data_1);

    page* np = pin_page(pc, 1, header, node_ft);

    assert(p->data[PAGE_SIZE - 1] == UCHAR_MAX >> (CHAR_BIT / 2));
    assert(np->data[0] == 5);

    unsigned char* write_mask = calloc(2, sizeof(unsigned char));
    write_bits(pc, p, 0, 0, 2 * CHAR_BIT, write_mask);

    assert(p->data[0] == 0);
    assert(p->data[1] == 0);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, 0, test_case_write, write_mask);

    assert(p->data[0] == 252);
    assert(p->data[1] == 0);

    write_mask    = malloc(sizeof(unsigned char));
    write_mask[0] = UCHAR_MAX;
    write_bits(pc, p, 0, test_case_write, 3, write_mask);

    assert(p->data[0] == 255);
    assert(p->data[1] == 128);

    unpin_page(pc, 0, header, node_ft);
    unpin_page(pc, 1, header, node_ft);

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("test header page - write bits successful\n");
}

int
main(void)
{
    test_compare_bits();
    test_shift_bit_array();
    test_concat_bit_arrays();
    test_split_bit_array();
    test_read_bits();
    test_write_bits();

    return 0;
}
