#include "query/degree.h"

#include "access/operators.h"

#include <assert.h>
#include <stdio.h>

static const float TRUE_AVG_DEG_BOTH_LOW  = 2.4F;
static const float TRUE_AVG_DEG_BOTH_HIGH = 2.6F;

static const float TRUE_AVG_DEG_OUT_LOW  = 1.1F;
static const float TRUE_AVG_DEG_OUT_HIGH = 1.3F;

static const float TRUE_AVG_DEG_INC_LOW  = 1.1F;
static const float TRUE_AVG_DEG_INC_HIGH = 1.3F;

void
test_get_degree(void)
{
    in_memory_file_t* db = create_in_memory_file();

    printf("t1\n");
    in_memory_create_node(db);
    printf("t1\n");
    in_memory_create_node(db);
    printf("t1\n");
    in_memory_create_node(db);
    printf("t1\n");
    in_memory_create_node(db);
    printf("t1\n");

    in_memory_create_relationship(db, 0, 1);
    printf("t1\n");
    in_memory_create_relationship(db, 0, 2);
    printf("t1\n");
    in_memory_create_relationship(db, 0, 3);
    printf("t1\n");
    in_memory_create_relationship(db, 1, 0);
    printf("t1\n");

    assert(get_degree(db, 0, BOTH, NULL) == 4);
    printf("t2\n");
    assert(get_degree(db, 0, OUTGOING, NULL) == 3);
    printf("t3\n");
    assert(get_degree(db, 0, INCOMING, NULL) == 1);
    printf("t4\n");

    in_memory_file_destroy(db);
}

void
test_get_avg_degree(void)
{
    in_memory_file_t* db = create_in_memory_file();

    in_memory_create_node(db);
    in_memory_create_node(db);
    in_memory_create_node(db);
    in_memory_create_node(db);

    in_memory_create_relationship(db, 0, 1);
    in_memory_create_relationship(db, 0, 2);
    in_memory_create_relationship(db, 0, 3);
    in_memory_create_relationship(db, 1, 0);
    in_memory_create_relationship(db, 2, 3);

    float avg_deg = get_avg_degree(db, BOTH, NULL);
    assert(avg_deg > TRUE_AVG_DEG_BOTH_LOW && avg_deg < TRUE_AVG_DEG_BOTH_HIGH);

    avg_deg = get_avg_degree(db, OUTGOING, NULL);
    assert(avg_deg > TRUE_AVG_DEG_OUT_LOW && avg_deg < TRUE_AVG_DEG_OUT_HIGH);

    avg_deg = get_avg_degree(db, INCOMING, NULL);
    assert(avg_deg > TRUE_AVG_DEG_INC_LOW && avg_deg < TRUE_AVG_DEG_INC_HIGH);

    assert(get_avg_degree(db, OUTGOING, NULL)
           == get_avg_degree(db, INCOMING, NULL));

    in_memory_file_destroy(db);
}

void
test_get_min_degree(void)
{
    in_memory_file_t* db = create_in_memory_file();

    in_memory_create_node(db);
    in_memory_create_node(db);
    in_memory_create_node(db);
    in_memory_create_node(db);

    in_memory_create_relationship(db, 0, 1);
    in_memory_create_relationship(db, 0, 2);
    in_memory_create_relationship(db, 0, 3);
    in_memory_create_relationship(db, 1, 0);
    in_memory_create_relationship(db, 2, 3);

    assert(get_min_degree(db, OUTGOING) == 0);
    assert(get_min_degree(db, BOTH) == 2);
    assert(get_min_degree(db, INCOMING) == 1);

    in_memory_file_destroy(db);
}

void
text_get_max_degree(void)
{
    in_memory_file_t* db = create_in_memory_file();

    in_memory_create_node(db);
    in_memory_create_node(db);
    in_memory_create_node(db);
    in_memory_create_node(db);

    in_memory_create_relationship(db, 0, 1);
    in_memory_create_relationship(db, 0, 2);
    in_memory_create_relationship(db, 0, 3);
    in_memory_create_relationship(db, 1, 0);
    in_memory_create_relationship(db, 2, 3);

    assert(get_max_degree(db, OUTGOING) == 3);
    assert(get_max_degree(db, BOTH) == 4);
    assert(get_max_degree(db, INCOMING) == 2);

    in_memory_file_destroy(db);
}

int
main(void)
{
    test_get_degree();
    printf("t1\n");
    test_get_avg_degree();
    printf("t2\n");
    test_get_min_degree();
    printf("t3\n");
    text_get_max_degree();
    printf("t4\n");

    printf("Tested degree functions successfully!\n");
}
