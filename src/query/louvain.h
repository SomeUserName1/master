#ifndef PARTITION_H
#define PARTITION_H

#define _GNU_SOURCE

#include "../access/in_memory_file.h"

static const double MIN_IMPROVEMENT = 0.005F;

typedef struct
{
    in_memory_file_t* graph;
    double m2;
} louvain_graph_t;

typedef struct
{
    // size of the partition
    unsigned long size;
    // community to which each node belongs
    unsigned long* node_to_com;
    // in and tot values of each node to compute modularity
    double* in;
    double* tot;
    // utility arrays to find communities adjacent to a node
    // communities are stored using three variables
    // - neighCommWeights: stores weights to communities
    // - neighCommPos: stores list of neighbor communities
    // - neighCommNb: stores the number of neighbor communities
    double* neigh_com_weights;
    unsigned long* neigh_com_pos;
    unsigned long neigh_com_nb;
    // TODO add pointer to finer and coarser partition
} louvain_partition_t;

louvain_graph_t*
louvain_graph_init(in_memory_file_t* db);

void
louvain_graph_destroy(louvain_graph_t* g);

/**
  Creates a louvain partition from a graph and initializes it
  @param g The graph for which a partition has to be createed
  @return The louvain partition
  */
louvain_partition_t*
create_louvain_partition(louvain_graph_t* g);

/**
  Frees a louvain partition and all pointers attached to it
  @param p The Louvain partition
  @return nothing
  */
void
louvain_part_destroy(louvain_partition_t* p);

// updates a given partition with the current Louvain partition
void
update_partition(louvain_partition_t* p,
                 unsigned long* part,
                 unsigned long size);

/**
 * generates the binary graph of communities as computed by one_level
 * Return the meta graph induced by a partition of a graph
 * See Louvain article for more details
 */
louvain_graph_t*
louvain_partition_to_graph(louvain_partition_t* p, louvain_graph_t* g);

int
compare_by_partition(const void* a, const void* b, void* array2);

unsigned long*
sort_by_partition(unsigned long* part, unsigned long size);

void
init_neighbouring_communities(louvain_partition_t* p);
/**
  Computes the set of neighboring communities of node
  @param p The Louvain partition
  @param g the partitionned graph
  @param node The node whose neighbor communities must be computed
  @return Nothing
  */
void
get_neighbouring_communities(louvain_partition_t* p,
                             louvain_graph_t* g,
                             unsigned long node,
                             bool all);

double
degree_weighted(louvain_graph_t* g, unsigned long node);

/**
  Returns the weight of the self-loop of a node
  Assumes that there is at most one self node for a given node
  @param graph The graph to be displayed
  @param node The node whose self-loop weight must be calculated
  @return the self-loop weight
  */
double
selfloop_weighted(louvain_graph_t* g, unsigned long node);

/**
  Computes modularity of the given partition
  @param p The Louvain partition
  @param g The partitionned graph
  @return The modularity of the partition
  */
double
compute_modularity(louvain_partition_t* p, louvain_graph_t* g);

/**
  Computes one level of Louvain (iterations over all nodes until no improvement)
  @param p The Louvain partition
  @param g The partitionned graph
  @param minImprovement The minimal improvement under which the process stops
  @return the increase of modularity during the level
  */
double
louvain_one_level(louvain_partition_t* p, louvain_graph_t* g);

/**
  Computes a partition with the Louvain method
  @param g The graph to be partitionned
  @param part The final partition
  @return nothing
  */
unsigned long*
louvain(in_memory_file_t* db);

#endif
