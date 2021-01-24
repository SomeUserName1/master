#ifndef PARTITION_H
#define PARTITION_H

#define _GNU_SOURCE

#include "../access/in_memory_file.h"

#define NNODES 10000000 //maximum number of nodes in the input graph: used for memory allocation, will increase if needed
#define NLINKS2 8
#define MIN_IMPROVEMENT 0.005

typedef struct {
    unsigned long n;//number of nodes
    unsigned long long e;//number of edges
    unsigned long long emax;//max number of edges
    unsigned long long *cd;//cumulative degree cd[0]=0 length=n+1
    unsigned long *adj;//concatenated lists of neighbors of all node
    long double *weights;//concatenated lists of weights of neighbors of all nodes
    long double totalWeight;//total weight of the links
    unsigned long *map;//map[u]=original label of node u
} adjlist;

typedef struct {
    in_memory_file_t* graph;
    long double total_weight;
    unsigned long* map;
} louvain_graph_t;

typedef struct {
    // size of the partition
    unsigned long size;

    // community to which each node belongs
    unsigned long* node_to_com;

    // in and tot values of each node to compute modularity
    long double* in;
    long double* tot;

    // utility arrays to find communities adjacent to a node
    // communities are stored using three variables
    // - neighCommWeights: stores weights to communities
    // - neighCommPos: stores list of neighbor communities
    // - neighCommNb: stores the number of neighbor communities
    long double* neigh_com_weights;
    unsigned long* neigh_com_pos;
    unsigned long neigh_com_nb;
} louvain_partition_t;


int compare_by_partition(const void *a, const void *b, void *array2);
unsigned long *sort_by_partition(unsigned long *part, unsigned long size);

long double degree_weighted(louvain_graph_t* g, unsigned long node);
/**
  Returns the weight of the self-loop of a node
  Assumes that there is at most one self node for a given node
  @param graph The graph to be displayed
  @param node The node whose self-loop weight must be calculated
  @return the self-loop weight
  */
long double selfloop_weighted(louvain_graph_t* g, unsigned long node);

// updates a given partition with the current Louvain partition
unsigned long update_partition(louvain_partition_t* p, unsigned long* part, unsigned long size);
/**
 * generates the binary graph of communities as computed by one_level
 * Return the meta graph induced by a partition of a graph
 * See Louvain article for more details
 */
louvain_graph_t* louvain_partition_to_graph(louvain_partition_t* p, louvain_graph_t* g);


/**
  Creates a louvain partition from a graph and initializes it
  @param g The graph for which a partition has to be createed
  @return The louvain partition
  */
louvain_partition_t* create_louvain_partition(louvain_graph_t* g);
/**
  Frees a louvain partition and all pointers attached to it
  @param p The Louvain partition
  @return nothing
  */
void louvain_part_destroy(louvain_partition_t* p);

/**
  Removes a node from its community and update modularity
  @param p The Louvain partition
  @param g the partitionned graph
  @param node The node to be removed
  @param comm The community to which node belongs
  @param dnodecomm The weighted degree from node to comm
  @return nothing
  */
void louvain_part_remove_node(louvain_partition_t* p, louvain_graph_t* g, unsigned long node, unsigned long comm, long double dnodecomm);
/**
  Adds a node to a community and update modularity
  @param p The Louvain partition
  @param g the partitionned graph
  @param node The node to be added
  @param comm The community to which node must be added
  @param dnodecomm The weighted degree from node to comm
  @return nothing
  */
void louvain_part_insert_node(louvain_partition_t* p, louvain_graph_t* g, unsigned long node, unsigned long comm, long double dnodecomm);
/**
  Computes the increase of modularity if a node where to be added to a given community
  - Note that node itself is not usefull
  - Note that the increase is not the real increase but ensures that orders are respected
  @param p The Louvain partition
  @param g the partitionned graph
  @param comm The community that is modified
  @param dnodecomm The weighted degree from node to comm
  @param nodeDegree The weighted degree of the node
  @return nothing
  */
long double gain(louvain_partition_t* p, louvain_graph_t* g, unsigned long comm, long double dnodecomm, long double d_node);
/**
  Computes modularity of the given partition
  @param p The Louvain partition
  @param g The partitionned graph
  @return The modularity of the partition
  */
long double compute_modularity(louvain_partition_t* p, louvain_graph_t* g);
void init_neighbouring_communities(louvain_partition_t* p);
/**
  Computes the set of neighboring communities of node
  @param p The Louvain partition
  @param g the partitionned graph
  @param node The node whose neighbor communities must be computed
  @return Nothing
  */
void get_neighbouring_communities(louvain_partition_t* p, louvain_graph_t* g, unsigned long node);
/**
  Same behavior as neighCommunities except:
  - self loop are counted
  - data structure if not reinitialised
  */
void get_neighbouring_communities_all(louvain_partition_t* p, louvain_graph_t* g, unsigned long node);
/**
  Computes one level of Louvain (iterations over all nodes until no improvement)
  @param p The Louvain partition
  @param g The partitionned graph
  @param minImprovement The minimal improvement under which the process stops
  @return the increase of modularity during the level
  */

louvain_graph_t* louvain_graph_init(in_memory_file_t* db);

void louvain_graph_destroy(louvain_graph_t* g);

long double louvain_one_level(louvain_partition_t* p, louvain_graph_t* g);
/**
  Computes a partition with the Louvain method
  @param g The graph to be partitionned
  @param part The final partition
  @return nothing
  */
unsigned long* louvain(in_memory_file_t* db);

#endif
