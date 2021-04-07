# Master Thesis:  Locality Optimization for traversal-based queries on graph databases

Master Project at the Database and Information Sytems Group, supervised by Dr. Theodoros Chondrogiannis and Prof. Michael Grossniklaus. Static data locality optimizing storage scheme for graph databases.  

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/db98dfa832514fecb1829fd2aab68728)](https://www.codacy.com/gh/SomeUserName1/master/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SomeUserName1/master&amp;utm_campaign=Badge_Grade)  [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/SomeUserName1/master.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/SomeUserName1/master/context:cpp)  [![codecov](https://codecov.io/gh/SomeUserName1/master/branch/main/graph/badge.svg?token=EHBWYZ8HYP)](https://codecov.io/gh/SomeUserName1/master)  

## Timetable 
<p style="text-align: center;"><font size="20"><progress id="time" value="17" max="22"></progress></font></p>

<font size="1">

| Mon  	| Tue  	| Wed  	| Thu  	| Fri  	| Sat  	| Sun  	|
|---	|---	|---	|---	|---	|---	|---	|
|22.3  :ballot_box_with_check: | 23.3 :ballot_box_with_check:   |  24.3 :ballot_box_with_check:	|   25.3 :ballot_box_with_check: |   26.3 :ballot_box_with_check:	|  27.3 :ballot_box_with_check: 	| 28.3  :ballot_box_with_check:	|
|  29.3 :ballot_box_with_check: | 30.3  :ballot_box_with_check: | 31.3 :ballot_box_with_check: 	| 01.4 :ballot_box_with_check: |  02.4 :ballot_box_with_check: | 03.4 :ballot_box_with_check: | 04.4 :ballot_box_with_check: | 
| 05.4 :ballot_box_with_check: 	| 06.04 :ballot_box_with_check:	|  07.4 :ballot_box_with_check: | 08.4 :computer: evaluation code  	| 09.4 :pencil: Edit with Theodoros & Stephans feedback 	  | 10.4 :computer: run & visualize experiments 	|  11.4 :pencil: Results & Conclusion 	|
| 12.4 :pencil: Proof reading & layout  	|  13.4 :pencil: Proof reading, layout 	| 14.4 :printer: to online printing service before 9am   	| 13.04 :watch:   	|   16.4 :dart: Thesis submission 	| 17.4 :beers:  	|   18.4 :beers:	|
| 19.4  :bar_chart: 	|   20.4 :bar_chart:	|   21.4 :speaking_head:	|   22.4 :bar_chart:	|   23.4 :speaking_head:	|  24.4 :sleeping:	|   25.4 :sleeping:	|
| 26.4  :bar_chart: 	|   27.4 :speaking_head: |  28.4  :speaking_head:	|   29.4 :speaking_head:	|   30.4 :dart: Defense	:checkered_flag: |  01.5 :beers:	|   02.5 :beers:	|

:pencil: = Writing thesis  
:computer: = write code  
:floppy_disk: = Buffer for debugging and rerunning experiments  
:bar_chart: = create slides  
:speaking_head: = practice  

</font>

## Thesis 

<p style="text-align: center;"><font size="20"><progress id="write" value="25" max="28">Thesis</progress></font></p>

### Results & Figures
  - [ ] Theodoros & Stephans Feedback
  - [ ] Results
  - [ ] Conclusion

### Done
**Abstract, Introduction, Conclusion, Achknowledgement ~ 3P**
  - [x] Acknowledgements
  - [x] Abstract
  - [x] Introduction
  - [x] Conclusions

**Evaluation 1 P**
  - [x] Implementation 
  - [x] Queries & data sets

**Method ~ 2 P.**
  - [x] Louvain Customizations: Formation & Ordering for nodes
  - [x] Formation & ordering for edges
  - [x] Incidence List rearrangement

**Graphs ~ 22 P.**  
  - [x] Definitions ~ 2 P
  - [x] Data structures ~ 5.5 P.
  - [x] traversal algos 
  - [x] Shortest path algos
  - [x] KL 
  - [x] Multilevel 
  - [x] Louvain

**Databases**
  - [x] Database Arch. ~ 3 P.
  - [x] Property Graph Model ~ 1.5 P.
  - [x] Neo4J & Index-free Incidence List ~ 5 P.
  
**Problem Def. ~ 5 P.**  
  - [x] Locality ~ 2 P.
  - [x] Problem definition ~ 0.5 P.
  - [x] Example ~ 2 P.

**Related Work ~ 6 P.**  
  - [x] G-Store
  - [x] ICBL
  - [x] Bondhu


## Implementation
<p style="text-align: center;"><font size="20"><progress id="file" value="24" max="26">Implementation</progress></font></p>  

- [ ] Evaluation code for ICBL, G-Store, Louvain, incidence list reordering
- [ ] Plot results

### Coverage
<p style="text-align: center;"><font size="14"><progress id="file" value="3376" max="4092">Coverage</progress></font></p>  

### Done
  - [x] Record structures  
  - [x] Data Structures (hash table, array list, queue)
  - [x] SNAP importer
  - [x] In Memory Access Layer 
  - [x] BFS
  - [x] DFS
  - [x] IDs to Pages to IOs computation
  - [x] Lovaine Method
  - [x] Random Walk
  - [x] Dijkstra
  - [x] A\*
  - [x] ALT
  - [x] G-Store
  - [x] ICBL
  - [x] Debug & Test Data structures & Record structures
  - [x] Debug & Test Access & Importer
  - [x] Debug & Test Louvain
  - [x] Debug & Test Queries
  - [x] Debug & Test ICBL
  - [x] Debug & Test G-Store
  - [x] Record remapping vertices & relationships
  - [x] Incidence List Reordering
  - [x] Debug & Test ids_to_io, remap records, reorder incidence list
  

## Future Improvements
### Job 
  - [ ] thread-safe BSD-style data structures (to avoid wrapper code) or use C++ or Rust
  - [ ] IO
  - [ ] Cache
  - [ ] Non-consecutive IDs & support deletions
  - [ ] Documentation
  - [ ] Dynamic Reorganization Impl. & Paper
  - [ ] Adapt louvain w RCM

### Additional stuff
  - [ ] Alternative record layouts (nodes + adj list in same file)
  - [ ] Thread-safety
  - [ ] Transactions/MVCC using git-like transaction logs
  - [ ] Distributed
  - [ ] Multi-Model
