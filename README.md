# Master Thesis:  Locality Optimization for traversal-based queries on graph databases

Dynamic record locality optimizing storage scheme for graph databases.  

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/db98dfa832514fecb1829fd2aab68728)](https://www.codacy.com/gh/SomeUserName1/master/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SomeUserName1/master&amp;utm_campaign=Badge_Grade)  [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/SomeUserName1/master.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/SomeUserName1/master/context:cpp) [![codecov](https://codecov.io/gh/SomeUserName1/master/branch/main/graph/badge.svg?token=EHBWYZ8HYP)](https://codecov.io/gh/SomeUserName1/master)  

## Timeline  
<p style="text-align: center;"><font size="20"><progress id="time" value="3" max="62"></progress></font></p>

- 31st July: Generic Data structures, Caching, Disk-based IO, Code Comments  
- Early Spetember: G-Store data format converter, Adaptive method
- 1st October: Paper


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
  - [ ] Heap file
  - [ ] Queries
  - [ ] layout
       => End of July  
  - [ ] Dynamic Reorganization Impl.
  - [ ] Gorder integration

#### Brainstorm
  - Access-History Graph
  - Extend Gorder's loss function to take HAG into account
  - Adapt louvain w. RCM

### Coverage
<p style="text-align: center;"><font size="14"><progress id="file" value="1435" max="1710">Coverage</progress></font></p>  

#### Test Cases
  - [ ] Compare reorganization of simulated and disk-based
  - [ ] Compare IOs of query on data set order
  
### Future Work
  - [ ] Hop labeling scheme: Use existing impl.
  - [ ] bulk ops
  - [ ] Alternative record layouts (nodes + adj list in same file)
  - [ ] thread-safe data structures
  - [ ] Transactions/MVCC using git-like transaction logs
  - [ ] Distributed
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


# Short-term TODO:
- [ ] Adjust record IDs when swapping pages
- [ ] CRUD
