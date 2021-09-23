# Graph Order Evaluation Database
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/db98dfa832514fecb1829fd2aab68728)](https://www.codacy.com/gh/SomeUserName1/master/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SomeUserName1/master&amp;utm_campaign=Badge_Grade)  [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/SomeUserName1/master.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/SomeUserName1/master/context:cpp) [![codecov](https://codecov.io/gh/SomeUserName1/master/branch/main/graph/badge.svg?token=YIBICJOF1R)](https://codecov.io/gh/SomeUserName1/master) 

The software in this repository provides an evaluation environment to experiment with the order of the graph as it is stored on disk.  
It consists of the low level components of a database with extended logging with respect to IO, several traversal-based queries, an importer for certain SNAP datasets and  utilities to change the order of the graph on disk.

## Architecture \& Source Code Organization


## Dependencies
- Curl
- Zlib



## Building
Create a build directory
```
mkdir build && cd build
```
Generate Makefiles
```
cmake .. -DCMAKE_BUILD_TYPE=DEBUG
```
or
```
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
```
Compile the source code 
```
make
```

## Testing
After building type 
```
make test
```
To generate coverage reports run
```
./../scripts/coverage.zsh
```
You can then view the report in the web browser of you choice, e.g. with firefox
```
firefox coverage/report.html&
```

## Running
- Sample(s)
- High level API overview: import, CRUD, get_nodes, expand, queries, reorder
- How to set buffer size & so on

## Limitations
- IDs, max, id handling - internal not adjustable to some degree, gaps on page boundaries, no shrink or delete page, no transactions, no concurrency, no query language and processor, no ACID, no properties
- File limits
- Neo4J design choices


## TODOs
### Documentation 
- [x] SRS
- [x] SDD
- [x] Copyright & License
- Code Comments
    - [ ] data-struct
    - [ ] io
    - [ ] cache
    - [ ] access
    - [ ] query
    - [ ] layout
- [ ] README
- [ ] Presentation
- [ ] Update SDD

### Implementation
#### Source
- [x] Fixup Includes
- [x] Marko-based genereic data structures
- [x] Disk-based IO
- [x] Cache
- [x] Heap file
- [x] Queries 
- [x] Weights for snap importer
- [x] logging
- [x] layout 
- [x] Filter/find by string id
- [x] log both string and internal id
- [ ] sample main file
- [ ] Make coverage script bash compatible

#### Tests
- [x] data structures
- [x] io
- [x] cache
- [x] access  
- [x] queries  
- [ ] layout 
- [ ] Benchmark crud, expand, get nodes and compare to Neo4j

#### Future Work
  - System Catalog 
  - Iterator for get nodes and get relationships
  - Bulk ops
  - thread-safe data structures & locks
  - Transactions & transaction buffers
  - Pattern-based/Cypher-like QL & interpreter
  - Properties
  - Alternative record layouts (nodes + adj list in same file, dense & sparse matrices)
  - Distributed
  - MVCC 
  - Multi-Model

