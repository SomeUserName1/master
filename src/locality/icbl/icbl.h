#ifndef ICBL
#define ICBL
#include "../../access/in_memory_file.h"
#include "../../data-struct/dict_ul.h"
#include "../../record/node.h"

#define MEMORY (8589934592.0f)
static const float MIN_DIST_INIT_CENTERS = 0.1F;

typedef struct dendrogram
{
    unsigned long id;
    char*         label;
    unsigned long block_no;
    size_t        size;
    size_t        uncapt_s;

    union
    {
        unsigned long      node;
        struct dendrogram* dendro[2];
    } children;
} dendrogram_t;

size_t
get_num_steps(in_memory_file_t* db);

size_t
get_num_walks(in_memory_file_t* db);

void
insert_match(unsigned long* max_degree_nodes,
             unsigned long* max_degrees,
             unsigned long  node_id,
             unsigned long  degree,
             unsigned long  num_clusters);

float
weighted_jaccard_dist(dict_ul_ul_t* dif_set_a, dict_ul_ul_t* dif_set_b);

void
identify_diffustion_sets(in_memory_file_t* db, dict_ul_ul_t** dif_sets);

bool
check_dist_bound(const size_t*  max_degree_nodes,
                 size_t         candidate,
                 unsigned long  num_found,
                 dict_ul_ul_t** dif_sets);

void
initialize_centers(in_memory_file_t* db,
                   unsigned long**   centers,
                   size_t            num_clusters,
                   dict_ul_ul_t**    dif_sets);

size_t
assign_to_cluster(size_t               num_nodes,
                  dict_ul_ul_t**       dif_sets,
                  unsigned long*       part,
                  const unsigned long* centers,
                  size_t               num_clusters);

void
update_centers(size_t               num_nodes,
               dict_ul_ul_t**       dif_sets,
               const unsigned long* part,
               unsigned long*       centers,
               size_t               num_clusters);

int
cluster_coarse(in_memory_file_t* db,
               dict_ul_ul_t**    dif_sets,
               unsigned long*    parts);

int
block_formation(in_memory_file_t*    db,
                dict_ul_ul_t**       dif_sets,
                const unsigned long* parts,
                dendrogram_t***      blocks,
                unsigned long*       block_count,
                dendrogram_t****     block_roots);

int
layout_blocks(in_memory_file_t* db,
              dendrogram_t***   blocks,
              unsigned long*    block_count,
              unsigned long*    partitions,
              dendrogram_t***   block_roots);

unsigned long*
icbl(in_memory_file_t* db);

#endif
