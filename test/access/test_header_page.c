#include "access/header_page.h"
#include "page.h"
#include "page_cache.h"

#include <assert.h>
#include <limits.h>

static const unsigned char test_number = 5;

void
test_compare_bits(void)
{
    const unsigned char test[1]   = { test_number };
    const unsigned char test_2[2] = { test_number, UCHAR_MAX };

    assert(compare_bits(test, 1 * CHAR_BIT, 1 << 2, 0));
    assert(compare_bits(test, 1 * CHAR_BIT, test_number, 0));
    assert(!compare_bits(test, 1 * CHAR_BIT, 1 << 4, 0));

    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX >> 4, 8));
    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 8));
    assert(compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 7));
    assert(!compare_bits(test_2, 2 * CHAR_BIT, UCHAR_MAX, 4));
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
}

void
test_concat_bit_arrays(void)
{
    unsigned char* test = calloc(2, sizeof(unsigned char));
    test[0]             = test_number;
    test[1]             = 0;

    unsigned char* test_1 = calloc(1, sizeof(unsigned char));
    test_1[0]             = UCHAR_MAX;

    unsigned char* result =
          concat_bit_arrays(test, test_1, 2 * CHAR_BIT, 1 * CHAR_BIT);

    printf("%u %u %u\n", result[0], result[1], result[2]);
    assert(result[0] == test_number);
    assert(result[1] == 0);
    assert(result[2] == UCHAR_MAX);

    free(result);
}
void
test_split_bit_array(void)
{
    unsigned char ar[3] = { UCHAR_MAX, 0, UCHAR_MAX };

    unsigned char** result =
          split_bit_array(ar, 3 * CHAR_BIT, 3 * CHAR_BIT / 2);

    assert(result[0][0] == (unsigned char)(UCHAR_MAX >> 4));
    assert(result[0][1] == (unsigned char)(UCHAR_MAX << 4));
    assert(result[1][0] == 0);
    assert(result[1][1] == UCHAR_MAX);

    free(result[0]);
    free(result[1]);
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

    allocate_pages(pc->pdb, node_file, CACHE_N_PAGES + 1);

    page* p = pin_page(pc, 0, node_file);
    write_uchar(p, 0, 1);

    unsigned char* bits = read_bits(pc, p, 0, CHAR_BIT - 1, 1);

    assert(bits[0] == 1);
    free(bits);

    write_uchar(p, 1, test_number);
    bits = read_bits(pc, p, 0, CHAR_BIT - 1, CHAR_BIT + 1);
    assert(bits[0] == 1);
    assert(bits[1] == test_number);

    free(bits);
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

    allocate_pages(pc->pdb, node_file, CACHE_N_PAGES + 1);

    page* p = pin_page(pc, 0, node_file);

    unsigned char data[1] = { 1 };
    write_bits(pc, p, 1, 1, 1, data);

    assert(p->data[1] == (1 << (CHAR_BIT - 1)));
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
