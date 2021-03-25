#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/htable.h"

#include <assert.h>
#include <stdio.h>

#define TEST_KEY (42)
#define TEST_VAL (777)
#define TEST_KEY_1 (11)
#define TEST_VALUE_1 (666)
#define PROGRESS_LINES (10000)

void
test_create_dict_ul_ul(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();

    assert(dict != NULL);
    assert(((htable_t*)dict)->num_used == 0);
    assert(((htable_t*)dict)->num_buckets == BUCKET_START);

    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_size(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();

    assert(((htable_t*)dict)->num_used == dict_ul_ul_size(dict));

    unsigned long key = TEST_KEY;
    unsigned long val = TEST_VAL;
    htable_insert(((htable_t*)dict), (void*)&key, (void*)&val);
    assert(((htable_t*)dict)->num_used == dict_ul_ul_size(dict));

    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_insert(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, TEST_KEY, TEST_VAL);
    assert(((htable_t*)dict)->num_used == 1);
    unsigned long key = TEST_KEY;
    assert(*((unsigned long*)htable_get_direct((htable_t*)dict, (void*)&key)) ==
           TEST_VAL);

    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_remove(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, TEST_KEY, TEST_VAL);
    assert(dict_ul_ul_size(dict) == 1);
    dict_ul_ul_remove(dict, TEST_KEY);
    assert(dict_ul_ul_size(dict) == 0);

    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_get(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, TEST_KEY, TEST_VAL);

    unsigned long* val = NULL;
    unsigned long key = TEST_KEY;
    dict_ul_ul_get(dict, key, &val);
    assert(*val == TEST_VAL);

    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_get_direct(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, TEST_KEY, TEST_VAL);

    assert(dict_ul_ul_get_direct(dict, TEST_KEY) == TEST_VAL);
    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_contains(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, TEST_KEY, TEST_VAL);

    assert(dict_ul_ul_contains(dict, TEST_KEY));
    dict_ul_ul_destroy(dict);
}

// Used to check for memory leaks
void
test_dict_ul_ul_destroy(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    const char* path = "/home/someusername/workspace_local/email_eu.txt";
    unsigned long int fromTo[2];
    int result;
    size_t lines = 1;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from");
        return;
    }
    result = fscanf(in_file, "%lu %lu\n", &fromTo[0], &fromTo[1]);

    while (result == 2) {
        if (lines % PROGRESS_LINES == 0) {
            printf("%s %lu\n", "Processed", lines);
        }

        if (dict_ul_ul_insert(dict, fromTo[0], fromTo[1])) {
            printf("Failes to insert\n");
            return;
        }

        result = fscanf(in_file, "%lu %lu\n", &fromTo[0], &fromTo[1]);
        lines++;
    }

    fclose(in_file);

    dict_ul_ul_destroy(dict);
}

void
test_dict_ul_ul_it(void)
{
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, TEST_KEY, TEST_VAL);
    dict_ul_ul_insert(dict, TEST_KEY_1, TEST_VALUE_1);

    assert(dict_ul_ul_contains(dict, TEST_KEY));
    assert(dict_ul_ul_contains(dict, TEST_KEY_1));

    dict_ul_ul_iterator_t* it = create_dict_ul_ul_iterator(dict);

    unsigned long k = 0;
    unsigned long v = 0;

    unsigned long* key = &k;
    unsigned long* value = &v;
    unsigned int count = 0;

    while (htable_iterator_next(
                 (htable_iterator_t*)it, (void**)&key, (void**)&value) > -1) {
        if (count == 0) {
            count++;
            assert(*key == TEST_KEY);
            assert(*value == TEST_VAL);
        }
        if (count == 1) {
            assert(*key == TEST_KEY_1);
            assert(*value == TEST_VALUE_1);
            count++;
        } else {
            assert(false);
        }
    }

    dict_ul_ul_iterator_destroy(it);
    dict_ul_ul_destroy(dict);
}

int
main(void)
{
    test_create_dict_ul_ul();
    test_dict_ul_ul_size();
    test_dict_ul_ul_insert();
    test_dict_ul_ul_remove();
    test_dict_ul_ul_get();
    test_dict_ul_ul_get_direct();
    test_dict_ul_ul_contains();
    test_dict_ul_ul_destroy();
    test_dict_ul_ul_it();
    printf("%s", "All tests for dict_ul_ul successfull\n");
}
