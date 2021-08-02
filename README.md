# Master Thesis:  Locality Optimization for traversal-based queries on graph databases

Dynamic record locality optimizing storage scheme for graph databases.  

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/db98dfa832514fecb1829fd2aab68728)](https://www.codacy.com/gh/SomeUserName1/master/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SomeUserName1/master&amp;utm_campaign=Badge_Grade)  [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/SomeUserName1/master.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/SomeUserName1/master/context:cpp) [![codecov](https://codecov.io/gh/SomeUserName1/master/branch/main/graph/badge.svg?token=EHBWYZ8HYP)](https://codecov.io/gh/SomeUserName1/master)  

## Documentation 

<p style="text-align: center;"><font size="20"><progress id="write" value="2" max="4">Documentation</progress></font></p>

- [x] SRS
- [x] SDD
- [ ] Code Comments
- [ ] Dev Manual


## Implementation
<p style="text-align: center;"><font size="20"><progress id="file" value="4" max="10">Implementation</progress></font></p>  

  - [x] Fixup Includes
  - [x] Marko-based genereic data structures
  - [x] Disk-based IO
  - [x] Cache
  - [x] Heap file
  - [x] Queries - use dicts instead of arrays!
  - [x] Weights for snap importer
  - [ ] logging
  - [ ] tests
       => Milestone 1  
  - [ ] layout
  - [ ] Data transformer (to adj list)
  - [ ] Gorder integration
  - [ ] Dynamic Reorganization Impl.
        => Milestone 2

#### Brainstorm
  - Access-History Graph
  - Dynamic: Extend Gorder's loss function to take HAG into account
  - Dynamic: RCM based on HAG
  - Static: Adapt louvain w. RCM

### Coverage
<p style="text-align: center;"><font size="14"><progress id="file" value="1435" max="1710">Coverage</progress></font></p>  

#### Test Cases
  - [ ] Compare reorganization of simulated and disk-based
  - [ ] Compare IOs of query on data set order
  
### Future Work
  - [ ] Iterator for get nodes and get relationships
  - [ ] Transaction/Intermediate Buffer
  - [ ] System Catalog (n\_slots from first 4 bytes of header and n\_nodes, n\_rels for now)
  - [ ] bulk ops
  - [ ] Hop labeling scheme: Use existing impl.
  - [ ] Alternative record layouts (nodes + adj list in same file)
  - [ ] thread-safe data structures
  - [ ] Transactions
  - [ ] Distributed
  - [ ] MVCC using git-like transaction logs
  - [ ] Multi-Model

## Meeting 1
- Goal: Paper  
	- Intro
	- X
	- Evaluation
	- Conclusion
	
- Deliverables: Test bed for method
	+ Disk-based
	+ G-Store format converter
	+ Schema (?)
	+ Evaluation: Run n% of queries, reorganize, run remaining 1-n%
	
- Keep using Neo4J like Gedik in the very back of our minds


## Meeting 2
- Caching in minimal form, non-consecutive IDS & deletions to be supported: 
    Otherwise the piece of software is too far from an actual database.
- By End of Semester/July: Measure number of actual disk IOs for the default/dataset order layout 

## Meeting 3
- labels stored within the nodes & rel struct
- Algorithms: The ones that are implemented + hop labeling
- Out of memory during transaction (e.g. BFS on live journal): Assume everything fits in memory



## Meeting 5 
- Leave louvain aside for now 
- Probably need in-memory graph, adjust in\_memory\_file
- Logging of 
    + algos => nodes & rels on algo level
    + CRUD => nodes & rels on system level
    + pages => un/pin in page cache
    + IOs => read/write page on disk files; 
- Intermediate/Transaction memory => Just malloc for now
- System catalog => leave as numbers in structs for now
- Tests first 
- Write data transformer from index free incidence list to adjacency list
- Layout afterwards when method is impl.

## Meeting 6
- Q: How to log header pages on heap file level? at all? page-wise (same as pin/unpin), byte-wise? slot-wise?
