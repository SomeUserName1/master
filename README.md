# master
Master Project at the Database and Information Sytems Group, supervised by Dr. Theodoros Chondrogiannis and Prof. Michael Grossniklaus. Dynamic data locality optimizing storage scheme for graph databases.  

## Formalities
- [x] Master Thesis Registration: Last handing in date is the 25th of May 2021  
- [ ] Hand-in Thesis
- [ ] Register Colloquium

## Thesis  
#### Introduction: 
- [ ] Essay
- [ ] brief problem statement
- [ ] Organization
- [ ] Contributions!

#### Background
- [ ] Graphs & Property Graph Model
- [ ] Neo4J & Index-free Adjacency List
- [ ] Traversals, Patterns
- [ ] Locality & Measures
- [ ] Problem definition

#### Related Work: 
- [ ] DRAW and the like
- [ ] Relational DBs for graphs, triplet stores (locaility setting in those)
- [ ] Interaction graphs

#### Methods: 
- [ ] G-Store
- [ ] ICBL
- [ ] Louvain + customizations

#### Evaluation:
- [ ] Describe Implementation & Setup
- [ ] Insert like on dataset -> run queries & analyze block usage -> run method & reorganize -> run queries and analysis again. 
- [ ] Visualize.

#### Conclusion
- [ ] Summary
- [ ] Future Work

#### Others
- Title
- Acknowledgements
- Bibliography

## Implementation
- [x] Record structures  
- [x] Data Structures (hash table, array list, queue)
- [x] SNAP importer
- [x] In Memory Access Layer 
- [x] BFS
- [x] IDs to Pages to IOs computation
- [x] Lovaine Method
- [ ] WIP: G-Store  [Steinhaus, Olteanu]. (TODO refinement using propper KL & Testing)  
- [x] Random Walk
- [ ] WIP: ICBL [Yasar, Gedik]
- [ ] Record remapping
- [ ] Dijkstra
- [ ] A*
- [ ] ALT
- [ ] DFS
- [ ] Spreading Activation

## Future Improvements
### Implementation Features
- [ ] Use initial community detection + data access history graph to update order subsequently based on queries 
- [ ] IO
- [ ] Cache  
- [ ] __DOCUMENTATION__
- [ ] __More Tests__
- [ ] __BSD-style data structures__ (to avoid wrapper code)
- [ ] Drop consecutive index assumptions & support deletions
- [ ] Alternative record layouts (nodes + adj list in same file)
- [ ] Transactions/MVCC using git-like transaction logs (aka a block chain without economics and marketing bs)
- [ ] Distribution
- [ ] Multi-Model Rel


### Additional Documentation
- [x] Write Neo4J Storage & Memory Design doc: Intro, Record Formats, BFS example   
- [ ] Write Neo4J Storage & Memory Design doc: Page Cache (Cursors & Caches plain what's on disk), IO parts  
