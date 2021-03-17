# master
Master Project at the Database and Information Sytems Group, supervised by Dr. Theodoros Chondrogiannis and Prof. Michael Grossniklaus. Dynamic data locality optimizing storage scheme for graph databases.  

## Formalities
- [x] Master Thesis Registration: Last handing in date is the 25th of May 2021  
- [ ] Hand-in Thesis
- [ ] Register Colloquium

## Thesis  
#### Introduction ~ 2 P. 
- [ ] Essay ~ 1 P.
- [ ] brief problem statement ~ 0.3 P.
- [ ] Organization ~ 0.3 P.
- [ ] Contributions! ~ 0.3 P.

#### Background ~ 21 P.
- [-] Low-level Database Arch. ~ 2.5 P.
- [x] Graphs ~ 2.5 P
- [x] Data structures ~ 6 P.
- [x] traversal-based algos ~ 5 P.
- [x] Property Graph Model ~ 1 P.
- [-] Neo4J & Index-free Adjacency List ~ 4 P.

#### Problem Def. ~ 5 P.
- [ ] Locality & Measures
- [ ] Problem definition
- [ ] Example

#### Related Work ~ 5 P.
- [ ] G-Store
- [ ] ICBL

#### Method ~ 5 P.
- [ ] Louvain + customizations
- [ ] Partition ordering
- [ ] Incidence List rearrangement

#### Evaluation ~ 15 P.
- [ ] Describe Implementation & Setup ~ 5 P.
- [ ] Results & Visualize. ~ 10 P.

#### Conclusion ~ 1 P.
- [ ] Summary ~ 0.5 P.
- [ ] Future Work ~ 0.5 P.

#### Others
- [x] Title page
- [ ] Acknowledgements
- [ ] Bibliography

**Overall 48 P. Contents + 10 P. Title, TOC, Abstract, Bibliography --> 60P.**

## Implementation
- [x] Record structures  
- [x] Data Structures (hash table, array list, queue)
- [x] SNAP importer
- [x] In Memory Access Layer 
- [x] BFS
- [x] DFS
- [x] IDs to Pages to IOs computation
- [x] Lovaine Method
- [x] Random Walk
- [ ] Dijkstra
- [ ] A\*
- [ ] ALT
- [-] G-Store  [Steinhaus, Olteanu]. (TODO refinement using propper KL & Testing)  
- [-] ICBL [Yasar, Gedik]
- [ ] Record remapping
- [ ] Incidence List Reordering


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
