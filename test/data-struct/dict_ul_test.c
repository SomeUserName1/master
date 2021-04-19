#include "../../src/data-struct/dict_ul.h"
#include "../../src/data-struct/htable.h"

#include <assert.h>
#include <stdio.h>

#include "../../src/access/in_memory_file.h"
#include "../../src/import/snap_importer.h"
#include "../../src/record/node.h"
#include "../../src/record/relationship.h"

#define TEST_KEY       (42)
#define TEST_VAL       (777)
#define TEST_KEY_1     (11)
#define TEST_VALUE_1   (666)
#define PROGRESS_LINES (10000)
#define DICT_ITER_REP  (1)

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
    assert(*((unsigned long*)htable_get_direct((htable_t*)dict, (void*)&key))
           == TEST_VAL);

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
    unsigned long  key = TEST_KEY;
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
    dict_ul_ul_t*     dict = create_dict_ul_ul();
    const char*       path = "/home/someusername/workspace_local/email_eu.txt";
    unsigned long int fromTo[2];
    int               result;
    size_t            lines = 1;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from\n");
        return;
    }

    do {
        result = fscanf(in_file, "%lu %lu\n", &fromTo[0], &fromTo[1]);

        if (dict_ul_ul_insert(dict, fromTo[0], fromTo[1])) {
            printf("Failes to insert\n");
            assert(false);
        }

        lines++;
    } while (result == 2);

    fclose(in_file);

    dict_ul_ul_print(dict);

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

    unsigned long* key   = NULL;
    unsigned long* value = NULL;

    while (dict_ul_ul_iterator_next(it, &key, &value) > -1) {
        if (*key == TEST_KEY || *key == TEST_KEY_1) {
            if (*key == TEST_KEY) {
                assert(*key == TEST_KEY);
                assert(*value == TEST_VAL);
            } else if (*key == TEST_KEY_1) {
                assert(*key == TEST_KEY_1);
                assert(*value == TEST_VALUE_1);
            } else {
                printf("key: %lu\n", *key);
                assert(false);
            }
        }
    }

    dict_ul_ul_iterator_destroy(it);
    dict_ul_ul_destroy(dict);
}

void
test_htable_iter(void)
{
    dict_ul_ul_t*     dict = create_dict_ul_ul();
    const char*       path = "/home/someusername/workspace_local/email_eu.txt";
    unsigned long int fromTo[2];
    int               result;
    size_t            lines = 1;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from\n");
        return;
    }

    do {

        result = fscanf(in_file, "%lu %lu\n", &fromTo[0], &fromTo[1]);

        if (dict_ul_ul_insert(dict, fromTo[0], fromTo[1])) {
            printf("Failes to insert\n");
            assert(false);
        }

        lines++;
    } while (result == 2);
    fclose(in_file);

    htable_iterator_t* it = create_htable_iterator((htable_t*)dict);

    void*  key   = NULL;
    void*  value = NULL;
    size_t size  = dict_ul_ul_size(dict);

    while (htable_iterator_next(it, &key, &value) > -1) {
        dict_ul_ul_remove(dict, *((unsigned long*)key));
        size--;
        assert(size == dict_ul_ul_size(dict));
    }
    assert(dict_ul_ul_size(dict) == 0);

    htable_iterator_destroy(it);
    dict_ul_ul_destroy(dict);
}

void
test_create_dict_ul_int(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();

    assert(dict != NULL);
    assert(((htable_t*)dict)->num_used == 0);
    assert(((htable_t*)dict)->num_buckets == BUCKET_START);

    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_size(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();

    assert(((htable_t*)dict)->num_used == dict_ul_int_size(dict));

    unsigned long key = TEST_KEY;
    int           val = TEST_VAL;
    htable_insert(((htable_t*)dict), (void*)&key, (void*)&val);
    assert(((htable_t*)dict)->num_used == dict_ul_int_size(dict));

    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_insert(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    dict_ul_int_insert(dict, TEST_KEY, TEST_VAL);
    assert(((htable_t*)dict)->num_used == 1);
    unsigned long key = TEST_KEY;
    assert(*((int*)htable_get_direct((htable_t*)dict, (void*)&key))
           == TEST_VAL);

    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_remove(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    dict_ul_int_insert(dict, TEST_KEY, TEST_VAL);
    assert(dict_ul_int_size(dict) == 1);
    dict_ul_int_remove(dict, TEST_KEY);
    assert(dict_ul_int_size(dict) == 0);

    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_get(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    dict_ul_int_insert(dict, TEST_KEY, TEST_VAL);

    int*          val = NULL;
    unsigned long key = TEST_KEY;
    dict_ul_int_get(dict, key, &val);
    assert(*val == TEST_VAL);

    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_get_direct(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    dict_ul_int_insert(dict, TEST_KEY, TEST_VAL);

    assert(dict_ul_int_get_direct(dict, TEST_KEY) == TEST_VAL);
    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_contains(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    dict_ul_int_insert(dict, TEST_KEY, TEST_VAL);

    assert(dict_ul_int_contains(dict, TEST_KEY));
    dict_ul_int_destroy(dict);
}

// Used to check for memory leaks
void
test_dict_ul_int_destroy(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    const char*    path = "/home/someusername/workspace_local/email_eu.txt";
    unsigned long  from;
    int            to;
    int            result;
    size_t         lines = 1;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from\n");
        return;
    }

    do {
        if (lines % PROGRESS_LINES == 0) {
            printf("%s %lu\n", "Processed", lines);
        }

        result = fscanf(in_file, "%lu %i\n", &from, &to);

        if (dict_ul_int_insert(dict, from, to)) {
            printf("Failes to insert\n");
            assert(false);
        }

        lines++;
    } while (result == 2);

    fclose(in_file);

    dict_ul_int_print(dict);

    dict_ul_int_destroy(dict);
}

void
test_dict_ul_int_it(void)
{
    dict_ul_int_t* dict = create_dict_ul_int();
    dict_ul_int_insert(dict, TEST_KEY, TEST_VAL);
    dict_ul_int_insert(dict, TEST_KEY_1, TEST_VALUE_1);

    assert(dict_ul_int_contains(dict, TEST_KEY));
    assert(dict_ul_int_contains(dict, TEST_KEY_1));

    dict_ul_int_iterator_t* it = create_dict_ul_int_iterator(dict);

    unsigned long* key   = NULL;
    int*           value = NULL;

    bool first  = false;
    bool second = false;
    while (dict_ul_int_iterator_next(it, &key, &value) > -1) {
        if (*key == TEST_KEY || *key == TEST_KEY_1) {
            if (*key == TEST_KEY) {
                assert(*key == TEST_KEY);
                assert(*value == TEST_VAL);
                first = true;
            } else if (*key == TEST_KEY_1) {
                assert(*key == TEST_KEY_1);
                assert(*value == TEST_VALUE_1);
                second = true;
            } else {
                printf("key: %lu\n", *key);
                assert(false);
            }
        }
    }
    assert(first && second);

    dict_ul_int_iterator_destroy(it);
    dict_ul_int_destroy(dict);
}

void
test_create_dict_ul_node(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();

    assert(dict != NULL);
    assert(((htable_t*)dict)->num_used == 0);
    assert(((htable_t*)dict)->num_buckets == BUCKET_START);

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_size(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();

    assert(((htable_t*)dict)->num_used == dict_ul_node_size(dict));

    unsigned long key = TEST_KEY;
    node_t*       val = new_node();
    htable_insert(((htable_t*)dict), (void*)&key, (void*)val);
    assert(((htable_t*)dict)->num_used == dict_ul_node_size(dict));

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_insert(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();
    node_t*         val  = new_node();
    dict_ul_node_insert(dict, TEST_KEY, val);
    assert(((htable_t*)dict)->num_used == 1);
    unsigned long key = TEST_KEY;
    assert(node_equals((node_t*)htable_get_direct((htable_t*)dict, (void*)&key),
                       val));

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_remove(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();
    node_t*         val  = new_node();
    dict_ul_node_insert(dict, TEST_KEY, val);
    assert(dict_ul_node_size(dict) == 1);
    dict_ul_node_remove(dict, TEST_KEY);
    assert(dict_ul_node_size(dict) == 0);

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_get(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();
    node_t*         val  = new_node();
    dict_ul_node_insert(dict, TEST_KEY, val);

    node_t*       value = NULL;
    unsigned long key   = TEST_KEY;
    dict_ul_node_get(dict, key, &value);
    assert(node_equals(value, val));

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_get_direct(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();
    node_t*         val  = new_node();
    dict_ul_node_insert(dict, TEST_KEY, val);

    assert(node_equals(dict_ul_node_get_direct(dict, TEST_KEY), val));

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_contains(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();
    node_t*         val  = new_node();
    dict_ul_node_insert(dict, TEST_KEY, val);

    assert(dict_ul_node_contains(dict, TEST_KEY));

    dict_ul_node_destroy(dict);
}

// Used to check for memory leaks
void
test_dict_ul_node_destroy(void)
{
    dict_ul_node_t* dict = create_dict_ul_node();
    const char*     path = "/home/someusername/workspace_local/email_eu.txt";
    unsigned long   fromTo[2];
    int             result;
    size_t          lines = 1;
    node_t*         node;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from\n");
        return;
    }

    do {
        if (lines % PROGRESS_LINES == 0) {
            printf("%s %lu\n", "Processed", lines);
        }

        result = fscanf(in_file, "%lu %lu\n", &fromTo[0], &fromTo[1]);

        node = new_node();
        if (dict_ul_node_insert(dict, fromTo[0], node)) {
            printf("Failes to insert\n");
            assert(false);
        }

        lines++;
    } while (result == 2);

    fclose(in_file);

    dict_ul_node_print(dict);

    dict_ul_node_destroy(dict);
}

void
test_dict_ul_node_it(void)
{
    dict_ul_node_t* dict  = create_dict_ul_node();
    node_t*         node  = new_node();
    node_t*         node1 = new_node();
    dict_ul_node_insert(dict, TEST_KEY, node);
    dict_ul_node_insert(dict, TEST_KEY_1, node1);

    assert(dict_ul_node_contains(dict, TEST_KEY));
    assert(dict_ul_node_contains(dict, TEST_KEY_1));

    dict_ul_node_iterator_t* it = create_dict_ul_node_iterator(dict);

    unsigned long* key   = NULL;
    node_t*        value = NULL;

    while (dict_ul_node_iterator_next(it, &key, &value) > -1) {
        if (*key == TEST_KEY || *key == TEST_KEY_1) {
            if (*key == TEST_KEY) {
                assert(*key == TEST_KEY);
                assert(node_equals(value, node));
            } else if (*key == TEST_KEY_1) {
                assert(*key == TEST_KEY_1);
                assert(node_equals(value, node1));
            } else {
                printf("key: %lu\n", *key);
                assert(false);
            }
        }
    }

    dict_ul_node_iterator_destroy(it);
    dict_ul_node_destroy(dict);
}

void
test_create_dict_ul_rel(void)
{
    dict_ul_rel_t* dict = create_dict_ul_rel();

    assert(dict != NULL);
    assert(((htable_t*)dict)->num_used == 0);
    assert(((htable_t*)dict)->num_buckets == BUCKET_START);

    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_size(void)
{
    dict_ul_rel_t* dict = create_dict_ul_rel();

    assert(((htable_t*)dict)->num_used == dict_ul_rel_size(dict));

    unsigned long   key = TEST_KEY;
    relationship_t* val = new_relationship();
    htable_insert(((htable_t*)dict), (void*)&key, (void*)val);
    assert(((htable_t*)dict)->num_used == dict_ul_rel_size(dict));

    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_insert(void)
{
    dict_ul_rel_t* dict = create_dict_ul_rel();

    relationship_t* val = new_relationship();
    dict_ul_rel_insert(dict, TEST_KEY, val);
    assert(((htable_t*)dict)->num_used == 1);
    unsigned long key = TEST_KEY;
    assert(relationship_equals(htable_get_direct((htable_t*)dict, (void*)&key),
                               val));

    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_remove(void)
{
    dict_ul_rel_t*  dict = create_dict_ul_rel();
    relationship_t* val  = new_relationship();
    dict_ul_rel_insert(dict, TEST_KEY, val);
    assert(dict_ul_rel_size(dict) == 1);
    dict_ul_rel_remove(dict, TEST_KEY);
    assert(dict_ul_rel_size(dict) == 0);

    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_get(void)
{
    dict_ul_rel_t*  dict = create_dict_ul_rel();
    relationship_t* val  = new_relationship();
    dict_ul_rel_insert(dict, TEST_KEY, val);

    relationship_t* value = NULL;
    unsigned long   key   = TEST_KEY;
    dict_ul_rel_get(dict, key, &value);
    assert(relationship_equals(value, val));

    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_get_direct(void)
{
    dict_ul_rel_t*  dict = create_dict_ul_rel();
    relationship_t* val  = new_relationship();
    dict_ul_rel_insert(dict, TEST_KEY, val);

    assert(relationship_equals(dict_ul_rel_get_direct(dict, TEST_KEY), val));
    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_contains(void)
{
    dict_ul_rel_t*  dict = create_dict_ul_rel();
    relationship_t* val  = new_relationship();
    dict_ul_rel_insert(dict, TEST_KEY, val);

    assert(dict_ul_rel_contains(dict, TEST_KEY));

    dict_ul_rel_destroy(dict);
}

// Used to check for memory leaks
void
test_dict_ul_rel_destroy(void)
{
    dict_ul_rel_t*  dict = create_dict_ul_rel();
    const char*     path = "/home/someusername/workspace_local/email_eu.txt";
    unsigned long   fromTo[2];
    int             result;
    size_t          lines = 1;
    relationship_t* val;

    FILE* in_file = fopen(path, "r");
    if (in_file == NULL) {
        perror("Failed to open file to read from\n");
        return;
    }

    do {
        if (lines % PROGRESS_LINES == 0) {
            printf("%s %lu\n", "Processed", lines);
        }

        result = fscanf(in_file, "%lu %lu\n", &fromTo[0], &fromTo[1]);

        val = new_relationship();
        if (dict_ul_rel_insert(dict, fromTo[0], val)) {
            printf("Failes to insert\n");
            assert(false);
        }

        lines++;
    } while (result == 2);

    fclose(in_file);

    dict_ul_rel_print(dict);

    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_it(void)
{
    dict_ul_rel_t*  dict = create_dict_ul_rel();
    relationship_t* rel  = new_relationship();
    relationship_t* rel1 = new_relationship();
    dict_ul_rel_insert(dict, TEST_KEY, rel);
    dict_ul_rel_insert(dict, TEST_KEY_1, rel1);

    assert(dict_ul_rel_contains(dict, TEST_KEY));
    assert(dict_ul_rel_contains(dict, TEST_KEY_1));

    dict_ul_rel_iterator_t* it = create_dict_ul_rel_iterator(dict);

    unsigned long*  key   = NULL;
    relationship_t* value = NULL;

    while (dict_ul_rel_iterator_next(it, &key, &value) > -1) {
        if (*key == TEST_KEY || *key == TEST_KEY_1) {
            if (*key == TEST_KEY) {
                assert(*key == TEST_KEY);
                assert(relationship_equals(value, rel));
            } else if (*key == TEST_KEY_1) {
                assert(*key == TEST_KEY_1);
                assert(relationship_equals(value, rel1));
            } else {
                printf("key: %lu\n", *key);
                assert(false);
            }
        }
    }

    dict_ul_rel_iterator_destroy(it);
    dict_ul_rel_destroy(dict);
}

void
test_dict_ul_rel_iter_large(void)
{
    in_memory_file_t* db = create_in_memory_file();
    dict_ul_ul_destroy(
          import_from_txt(db, "/home/someusername/workspace_local/dblp.txt"));

    assert(db->node_id_counter == DBLP_NO_NODES);
    assert(db->rel_id_counter == DBLP_NO_RELS);

    dict_ul_rel_iterator_t* it = create_dict_ul_rel_iterator(db->cache_rels);

    unsigned long*  key   = NULL;
    relationship_t* value = NULL;
    unsigned long   counter;
    for (size_t i = 0; i < DICT_ITER_REP; ++i) {
        ((htable_iterator_t*)it)->idx = 0;
        counter                       = 0;
        while (dict_ul_rel_iterator_next(it, &key, &value) > -1) {
            counter++;
        }
        printf("counter: %lu, dblp rel %d\n", counter, DBLP_NO_RELS);
        assert(counter == DBLP_NO_RELS);
    }

    dict_ul_rel_iterator_destroy(it);
    in_memory_file_destroy(db);
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

    test_create_dict_ul_int();
    test_dict_ul_int_size();
    test_dict_ul_int_insert();
    test_dict_ul_int_remove();
    test_dict_ul_int_get();
    test_dict_ul_int_get_direct();
    test_dict_ul_int_contains();
    test_dict_ul_int_destroy();
    test_dict_ul_int_it();

    test_create_dict_ul_node();
    test_dict_ul_node_size();
    test_dict_ul_node_insert();
    test_dict_ul_node_remove();
    test_dict_ul_node_get();
    test_dict_ul_node_get_direct();
    test_dict_ul_node_contains();
    test_dict_ul_node_destroy();
    test_dict_ul_node_it();

    test_create_dict_ul_rel();
    test_dict_ul_rel_size();
    test_dict_ul_rel_insert();
    test_dict_ul_rel_remove();
    test_dict_ul_rel_get();
    test_dict_ul_rel_get_direct();
    test_dict_ul_rel_contains();
    test_dict_ul_rel_destroy();
    test_dict_ul_rel_it();

    test_dict_ul_rel_iter_large();

    printf("%s", "All tests for dict_ul successfull\n");
}
