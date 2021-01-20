#ifndef LOUVAIN_H
#define LOUVAIN_H

// louvain.c
// main and io methods
// readadj list: iterate over nodes and comput cumulative degrees, total weight (for unweighted 2*|edges|)
//      weights and edge = NULL

// struct.h
// edge, edgelist, adjlist
// edgelist additional fields: no nodes, no edges, max num edges, "map[u]=original label of node u"
// adjlist: no nodes, no edges, cumulative degree, concatenated lists of neighbouts of all node (?), weights, total weight, map as above

// partition.h
//  struct louvainPartition: size. community to which each node belongs, in and tot values of each node, per node weights to community, list of neighbour communities, number of neighbour comm.
//  functions:
//      init, 
//      louvain,
//      louvainComplete
//      degreeWeighted
//      selfloopWeighted
//      create & free louvain partition
//      modularity
//      neighCommunities
//      updateGraphPartition
//      louvainPartition2Graph
//      louvainOneLevel
//      

#endif
