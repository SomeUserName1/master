#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/htable.h"

#include <assert.h>
#include <stdio.h>

void test_create_dict_ul_ul(void) {
    dict_ul_ul_t* dict = create_dict_ul_ul();

    assert(dict != NULL);
    assert(((htable_t*) dict)->num_used == 0);
    assert(((htable_t*) dict)->num_buckets == BUCKET_START);

    dict_ul_ul_destroy(dict);
}

void test_dict_ul_ul_size(void) {
     dict_ul_ul_t* dict = create_dict_ul_ul();

    assert(((htable_t*) dict)->num_used == dict_ul_ul_size(dict));

    unsigned long key = 42;
    unsigned long val = 777;
    htable_insert(((htable_t*) dict), (void*) &key, (void*) &val);
    assert(((htable_t*) dict)->num_used == dict_ul_ul_size(dict));

    dict_ul_ul_destroy(dict);
}

void test_dict_ul_ul_insert(void) {
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, 42, 777);
    assert(((htable_t*) dict)->num_used == 1);
    unsigned long key = 42;
    assert(*((unsigned long*) htable_get_direct((htable_t*) dict, (void*) &key)) == 777);

    dict_ul_ul_destroy(dict);
}

void test_dict_ul_ul_remove(void) {
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, 42, 777);
    assert(dict_ul_ul_size(dict) == 1);
    dict_ul_ul_remove(dict, 42);
    assert(dict_ul_ul_size(dict) == 0);

    dict_ul_ul_destroy(dict);
}

void test_dict_ul_ul_get(void) {
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, 42, 777);

    unsigned long* val = NULL;
    unsigned long key = 42;
    dict_ul_ul_get(dict, key, &val);
    assert(*val == 777);

    dict_ul_ul_destroy(dict);
}

void test_dict_ul_ul_get_direct(void) {
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, 42, 777);

    assert(dict_ul_ul_get_direct(dict, 42) == 777);
    dict_ul_ul_destroy(dict);
}

void test_dict_ul_ul_contains(void) {
    dict_ul_ul_t* dict = create_dict_ul_ul();
    dict_ul_ul_insert(dict, 42, 777);

    assert(dict_ul_ul_contains(dict, 42));
    dict_ul_ul_destroy(dict);
}

int main(void) {
    test_create_dict_ul_ul();
    test_dict_ul_ul_size();
    test_dict_ul_ul_insert();
    test_dict_ul_ul_remove();
    test_dict_ul_ul_get();
    test_dict_ul_ul_get_direct();
    test_dict_ul_ul_contains();
    printf("%s", "All tests for dict_ul_ul successfull\n");
}
