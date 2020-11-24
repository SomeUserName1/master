# Literature Summary

## DRAW: Data gRouping AWare Placement Scheme With Interest Locality	
- Shall replace random placement of Hadoop
- "interest locality": Only sweep a part of a big data set
- Use log files to check data access
- Extracts optimal grouping and rearranges data placement
- Goal here is to distribute all files of interest evenly s.t. each node computes approx. the same amount of relevant data
- History Data Access Graph: Graph describing patterns among files/pages/blocks/entries. Connect all entries that get accessed sequentially by an edge, labeled with the query which caused the loading.
- Create sets of directly nodes connected by query
- Build matrix of co-occurrences in tasks per entry
- cluster the matrix, extract connected subgroups, evenly distribute members from those groups  
 
 
## G-Store: A Storage Manager for Graph Data
 - Use multi-level partitioning to place entries into pages 
 - Optimized for neighborhood queries, travlersals, shortest-path, ...
 - C++ Implementation; data access layer & storage layout layer
 - Partition nodes to pages & order pages; Both NP-Hard
 - Page Ordering: Minimum linear arrangement problem
 - TODO Algo
 
 - Storage-aware query optimization: Keep statistics on average amount of pages for a k-neigborhood for cost estimation
 - Query compilation based on Access layer gtaph traversal primitives
 - Secondary indices
 - Updates!: On insertion and removal adjust the schema, deal with "holes" [33]
 
 
 
##Idea Section
- Use multi-level partitioning as initial placement
- Refine using access history graph/access pattern graph
- 






Locality optimization for traversal-based queries on graph databases 