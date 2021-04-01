#include "../../src/query/degree.h"

#include "../../src/access/in_memory_file.h"

#include <assert.h>
#include <stdio.h>

static const float TRUE_AVG_DEG_BOTH_LOW = 2.4F;
static const float TRUE_AVG_DEG_BOTH_HIGH = 2.6F;

static const float TRUE_AVG_DEG_OUT_LOW = 1.1F;
static const float TRUE_AVG_DEG_OUT_HIGH = 1.3F;

static const float TRUE_AVG_DEG_INC_LOW = 1.1F;
static const float TRUE_AVG_DEG_INC_HIGH = 1.3F;

void
test_get_degree(void)
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

    assert(get_degree(db, 0, BOTH) == 4);
    assert(get_degree(db, 0, OUTGOING) == 3);
    assert(get_degree(db, 0, INCOMING) == 1);

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

    float avg_deg = get_avg_degree(db, BOTH);
    assert(avg_deg > TRUE_AVG_DEG_BOTH_LOW && avg_deg < TRUE_AVG_DEG_BOTH_HIGH);

    avg_deg = get_avg_degree(db, OUTGOING);
    assert(avg_deg > TRUE_AVG_DEG_OUT_LOW && avg_deg < TRUE_AVG_DEG_OUT_HIGH);

    avg_deg = get_avg_degree(db, INCOMING);
    assert(avg_deg > TRUE_AVG_DEG_INC_LOW && avg_deg < TRUE_AVG_DEG_INC_HIGH);

    assert(get_avg_degree(db, OUTGOING) == get_avg_degree(db, INCOMING));

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
    test_get_avg_degree();
    test_get_min_degree();
    text_get_max_degree();

    printf("Tested degree functions successfully!\n");
}
