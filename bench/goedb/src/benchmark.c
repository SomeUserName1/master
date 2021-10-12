#include <limits.h>
#include <time.h>

#include "access/heap_file.h"
#include "access/node.h"
#include "access/relationship.h"

static const size_t n_calls_per_fn = 100000;
static const size_t m_to_s         = 60;
static const size_t s_to_mus       = 1000000;
static const size_t ns_to_mus      = 1000;
static const size_t buf_sz         = 81920;

heap_file*
prepare(void)
{
    char* file_name = "bench";

    char* log_name_phf   = "log_bench_pdb";
    char* log_name_cache = "log_bench_pc";
    char* log_name_file  = "log_bench_hf";

    phy_database* pdb = phy_database_create(file_name, log_name_phf);

    page_cache* pc = page_cache_create(pdb, buf_sz / PAGE_SIZE, log_name_cache);

    heap_file* hf = heap_file_create(pc, log_name_file);

    return hf;
}

void
clean_up(heap_file* hf)
{
    page_cache*   pc  = hf->cache;
    phy_database* pdb = pc->pdb;
    heap_file_destroy(hf);
    page_cache_destroy(pc);
    phy_database_delete(pdb);
}

void
create_nodes(heap_file* hf, bool time)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total = 0;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        if (time) {
            timespec_get(&start, TIME_UTC);
        }

        create_node(hf, i, false);

        if (time) {
            timespec_get(&end, TIME_UTC);
            total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                      - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        }
    }
    if (time) {
        printf("Creating 15K nodes took %d M %d S. Average call takes %f "
               "mu "
               "s\n",
               (int)(total / s_to_mus / m_to_s),
               (int)((total / s_to_mus)
                     - (int)(total / s_to_mus / m_to_s) * m_to_s),
               (float)total / (float)n_calls_per_fn);
    }
}

void
create_rels(heap_file* hf, bool time)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total = 0;

    node_t*          node  = read_node(hf, 0, false);
    array_list_node* nodes = get_nodes(hf, false);
    unsigned long    next_id;
    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        next_id = array_list_node_get(nodes, i)->id;
        if (time) {
            timespec_get(&start, TIME_UTC);
        }

        create_relationship(hf, node->id, next_id, 1.0, i, false);

        if (time) {
            timespec_get(&end, TIME_UTC);
            total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                      - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        }
    }
    array_list_node_destroy(nodes);
    if (time) {
        printf("Creating 15K relationships took %d M %d S. Average call takes "
               "%f mu s\n",
               (int)(total / s_to_mus / m_to_s),
               (int)((total / s_to_mus)
                     - (int)(total / s_to_mus / m_to_s) * m_to_s),
               (float)total / (float)n_calls_per_fn);
    }
}

void
read_nodes(heap_file* hf)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total = 0;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        timespec_get(&start, TIME_UTC);

        read_node(hf, i, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
    }

    printf(
          "Reading 15K nodes took %d M %d S. Average call takes "
          "%f mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

void
read_rels(heap_file* hf)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total    = 0;
    size_t          cur_slot = 0;
    size_t          cur_page = 0;
    size_t          cur_addr;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        if (cur_slot >= UCHAR_MAX) {
            cur_slot = 0;
            cur_page++;
        }
        cur_addr = (cur_page << CHAR_BIT) | (cur_slot & UCHAR_MAX);
        timespec_get(&start, TIME_UTC);

        read_relationship(hf, cur_addr, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));

        cur_slot += NUM_SLOTS_PER_REL;
    }

    printf(
          "Reading 15K relationships took %d M %d S. Average call "
          "takes %f mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

void
update_nodes(heap_file* hf)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total = 0;

    node_t* node;
    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        node = read_node(hf, i, false);
        timespec_get(&start, TIME_UTC);

        node->label = n_calls_per_fn - i - 1;
        update_node(hf, node, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
    }

    printf(
          "Updating 15K nodes took %d M %d S. Average call takes %f mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

void
update_rels(heap_file* hf)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total    = 0;
    size_t          cur_slot = 0;
    size_t          cur_page = 0;
    size_t          cur_addr;

    relationship_t* rel;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        if (cur_slot >= UCHAR_MAX) {
            cur_slot = 0;
            cur_page++;
        }
        cur_addr = (cur_page << CHAR_BIT) | (cur_slot & UCHAR_MAX);
        rel      = read_relationship(hf, cur_addr, false);
        timespec_get(&start, TIME_UTC);

        rel->label = n_calls_per_fn - i - 1;
        update_relationship(hf, rel, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        cur_slot += NUM_SLOTS_PER_REL;
    }

    printf(
          "Updating 15K relationships took %d M %d S. Average call takes "
          "%f mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

void
delete_nodes(heap_file* hf, bool time)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total = 0;
    node_t*         node;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        node = read_node(hf, i, false);
        if (time) {
            timespec_get(&start, TIME_UTC);
        }

        delete_node(hf, node->id, false);

        if (time) {
            timespec_get(&end, TIME_UTC);
            total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                      - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        }
    }

    if (time) {
        printf("Deleting 15K nodes took %d M %d S. Average call takes %f mu "
               "s\n",
               (int)(total / s_to_mus / m_to_s),
               (int)((total / s_to_mus)
                     - (int)(total / s_to_mus / m_to_s) * m_to_s),
               (float)total / (float)n_calls_per_fn);
    }
}

void
delete_rels(heap_file* hf)
{
    struct timespec start;
    struct timespec end;
    unsigned long   total    = 0;
    size_t          cur_slot = 0;
    size_t          cur_page = 0;
    size_t          cur_addr;

    relationship_t* rel;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        if (cur_slot >= UCHAR_MAX) {
            cur_slot = 0;
            cur_page++;
        }
        cur_addr = (cur_page << CHAR_BIT) | (cur_slot & UCHAR_MAX);
        rel      = read_relationship(hf, cur_addr, false);
        timespec_get(&start, TIME_UTC);

        delete_relationship(hf, rel->id, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));

        cur_slot += NUM_SLOTS_PER_REL;
    }

    printf("Deleting 15K relationships took %lu M %lu S. Average call takes "
           "%f mu s\n",
           (unsigned long)(total / s_to_mus / m_to_s),
           (unsigned long)((total / s_to_mus)
                           - (unsigned long)(total / s_to_mus / m_to_s)
                                   * m_to_s),
           (float)total / (float)n_calls_per_fn);
}

void
bench_get_nodes(heap_file* hf)
{
    struct timespec  start;
    struct timespec  end;
    unsigned long    total = 0;
    array_list_node* nodes;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        timespec_get(&start, TIME_UTC);

        nodes = get_nodes(hf, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        array_list_node_destroy(nodes);
    }

    printf(
          "Calling 15K times get_nodes took %d M %d S. Average call takes "
          "%f mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

void
bench_get_rels(heap_file* hf)
{
    struct timespec          start;
    struct timespec          end;
    unsigned long            total = 0;
    array_list_relationship* rels;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        timespec_get(&start, TIME_UTC);

        rels = get_relationships(hf, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        array_list_relationship_destroy(rels);
    }

    printf(
          "Calling 15K times get_relationships took %d M %d S. Average call "
          "takes %f mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

void
bench_expand(heap_file* hf)
{
    struct timespec          start;
    struct timespec          end;
    unsigned long            total = 0;
    array_list_node*         nodes = get_nodes(hf, false);
    unsigned long            nid;
    array_list_relationship* rels;

    for (size_t i = 0; i < n_calls_per_fn; ++i) {
        nid = array_list_node_get(nodes, i)->id;
        timespec_get(&start, TIME_UTC);

        rels = expand(hf, nid, BOTH, false);

        timespec_get(&end, TIME_UTC);
        total += ((end.tv_sec * s_to_mus + end.tv_nsec / ns_to_mus)
                  - (start.tv_sec * s_to_mus + start.tv_nsec / ns_to_mus));
        array_list_relationship_destroy(rels);
    }

    printf(
          "Calling 15K times expand took %d M %d S. Average call takes %f "
          "mu s\n",
          (int)(total / s_to_mus / m_to_s),
          (int)((total / s_to_mus) - (int)(total / s_to_mus / m_to_s) * m_to_s),
          (float)total / (float)n_calls_per_fn);
}

int
main(void)
{
    heap_file* hf = prepare();
    create_nodes(hf, true);
    create_rels(hf, true);

    read_nodes(hf);
    read_rels(hf);

    update_nodes(hf);
    update_rels(hf);

    delete_nodes(hf, false);
    create_nodes(hf, false);
    create_rels(hf, false);

    delete_nodes(hf, true);

    create_nodes(hf, false);
    create_rels(hf, false);

    delete_rels(hf);

    create_rels(hf, false);

    bench_get_nodes(hf);
    bench_get_rels(hf);
    bench_expand(hf);

    clean_up(hf);
}
