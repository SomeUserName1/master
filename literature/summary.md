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
### Intro
 - Use multi-level partitioning to place entries into pages 
 - Optimized for neighborhood queries, travlersals, shortest-path, ...
 - C++ Implementation; data access layer & storage layout layer
 - Partition nodes to pages & order pages; Both NP-Hard
 - Page Ordering: Minimum linear arrangement problem

### Cost Model
C1: minimal linear arrangement  
$\sum_{\{u, v\} \in E} |\phi(\pi(u)) - \phi(\pi(v))|$ with $\pi$ partitioning function, $\phi$ permuatation.  

C2/C3: Partitioning  
Penalize adjacent vertices in distinct partitions.  
$\sum_{\{u, v\} \in E} w \delta_{uv}$ with $\delta_{uv} = 1$ if $\pi(u) \neq \pi(v)$  
  
Penalize edges between partitions.  
$\sum_{i < j} w \delta_{ij}$ with $\delta_{ij} = 1$ if $\exists \{u,v\} \in E: \pi(u) = i \wedge \pi(v) = j$  

As neighbouring vertices in distinct partitions $$\Rightarrow$$ edges between partitons, C2 implies C3.  
Useful for us, as C2 is a vertex placement cost and C3 is an edge placement cost.  
I.e. Adapt C3 s.t. edges between partitions correspond to relationships in the same adjacency list that are not in the same block.  
 
### Algo
1. Modified Heavy Edge Matching: HEM with parameters to set no. of matched vartices (standard 2) and maximum no. matched vartices (standard: infty. Here block size many, increasing). Done to mitigate decreasing aggregation factor btw iterations due to hub and spoke nodes.

2. Turn-arround: Assign partition numbers. For each CC with size > block size assign number. For rest assign same number until part > block size

3. Uncoarsening: 
    - Project: Derive first projection
    - Reorder: Swap partitions
    - Refine: Swap vertices between partitions


## Scalable layout of large graphs on disk [Yasar, Gedik]
### Intro
distributed due to MapReduce. Else same problem as with G-Store.


### Cost model
Block locality:  
- Conductance $C_D$:  
|Edges with only one vertex in block| / |edges in block|  
- Cohesiveness $C_H$:  
|Adjacent vertices in block| / |vertex pairs in block|

- Overall:
$L(B) = \sqrt{C_D (1 - C_H)}$

Ordering locality:  
$$R(B) = 1 - \frac{\sum_{v \in V_B} \sum_{u \in N_v} |\phi(u) - \phi(v)|}{|\text{blocks}| \sum_{v \in B} N_v}$$

### Algorithm
1. Indentify Diffusion sets: Execute t random walks of length l and capture the vertices the were visited along with how often they were visited.  
$t = \argmin_x f'(x) \geq 1$ with $f$ cdf of node degree.  
$l = 1 + \lceil ln(|V|)/k \rceil$

2. Coarse partitioning using k-Means based on weighted jaccard distance of diffusion sets  
Distance function: $J_w(u, v) = 1 - \frac{\sum_{x \in D_u \cap D_v} \min w_{x, D_u} w_{x, D_v}}{\sum_{x \in D_u \cup D_v} \max w_{x, D_u}, w_{x, D_v}}$

$k$ chosen based upon memory availability of node in cluster:  
$k = \lceil |\text{node in }B| \cdot |nodes| / \sqrt{0.8 |\text{Memory per comp. node}]|}$  
Adapt: memory = RAM  
"We produce new centers by counting the number of occurences of vertices in each cluster and keeping the most frequent ones"; I.e. most frequent node in diffusion set is centroid.  
Initial centers: Sort nodes by degree and pick those which are appart at least 0.9  

3. & 4. Hierarchical clustering of k partitions based on diffusion sets & assign labels
Bottom-up hierarchical clustering. Each vertex in own partition, merged by picking closest pair of part at each step (min weighted jaccard). 
After each of the k subgraphs has been clustered and labelled, the same procedure is applied between the components.

To be implemented:
- Random walk
- k-Medoid-like clustering with custom jaccard distance
- hierarchical clustering including labelling
- ordering of subgraphs based on dist.



 
## Idea Section
- Use multi-level partitioning/louvain as partitioning/clustering
- Use Uncoarsening phase/louvain partitions/block graph + dewey scheme + sth/n-body simulation to order
- Refine using access history graph/access pattern graph: Louvain + place communities close or n-body






Locality optimization for traversal-based queries on graph databases 
