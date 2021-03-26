# Master Thesis:  Locality Optimization for traversal-based queries on graph databases

Master Project at the Database and Information Sytems Group, supervised by Dr. Theodoros Chondrogiannis and Prof. Michael Grossniklaus. Dynamic data locality optimizing storage scheme for graph databases.  

## Formalities
- [x] Master Thesis Registration: Last handing in date is the 25th of May 2021  
- [ ] Hand-in Thesis: 16.04
- [ ] Register Colloquium: Form filled besides M.Scholl (TODO on submission day)

## Timetable
<p style="text-align: center;">
<progress id="time" value="3" max="26">Time</progress>



| Mon  	| Tue  	| Wed  	| Thu  	| Fri  	| Sat  	| Sun  	|
|---	|---	|---	|---	|---	|---	|---	|
|22.3  :ballot_box_with_check: | 23.3 :ballot_box_with_check:   |  24.3 :ballot_box_with_check:	|   25.3 :ballot_box_with_check: |   26.3 :pencil: Louvain, Multilevel p., KL	|  27.3 :pencil: Louvain Adaptions 	| 28.3 :pencil: Relationship block formation &ordering & incidence list sort   	|
|  29.3 :pencil: Implementation, Queries, data sets | 30.3 :pencil: Abstract, Introduction, Conclusion, Ackknowled   | 31.3 :computer: Tests & Debug Data struct  	| 01.4 :computer: Test & Debug Queries |  02.4  :computer: Tests & Debug Access, ICBL  	| 03.4  :computer: Test & Debug G-Store	| 04.4 :floppy_disk: Debugging  	|
| 05.4 :computer: Record Remapping Nodes  & rels  	| 06.04  :computer: remap rels & Incidence List sort 	|  07.4 :computer: adapt louvain 	| 08.4 :computer: adapt louvain  	| 09.4 :computer: evalutaion code   | 10.4 :computer: run Experiments 	|  11.4 :pencil: results 	|
| 12.4 :floppy_disk:   	| 13.04 :floppy_disk:  	| 14.4 :pencil: Proof reading & layout  	|  15.4 :pencil: Proof reading, layout, printing 	|  16.4 :dart: Thesis submission 	| 17.4 :beers:  	|   18.4 :beers:	|
| 19.4  :bar_chart: 	|   20.4 :bar_chart:	|   21.4 :speaking_head:	|   22.4 :bar_chart:	|   23.4 :speaking_head:	|  24.4 :sleeping:	|   25.4 :sleeping:	|
| 26.4  :bar_chart: 	|   27.4 :speaking_head: |  28.4  :speaking_head:	|   29.4 :speaking_head:	|   30.4 :dart: Defense	:checkered_flag: |  01.5 :beers:	|   02.5 :beers:	|
</p>

:pencil: = Writing thesis  
:computer: = write code  
:floppy_disk: = Buffer for debugging and rerunning experiments  
:bar_chart: = create slides  
:speaking_head: = practice  

## Thesis  
<p style="text-align: center;">
<progress id="write" value="4" max="7">Thesis</progress>
</p>

#### Method ~ 5 P.
- [ ] Louvain Customizations: Formation & Ordering for nodes
- [ ] Formation & ordering for edges
- [ ] Incidence List rearrangement

#### Evaluation ~ 15 P.
- [ ] Implementation 
- [ ] Queries & data sets
- [ ] Results & Visualize. ~ 10 P.

#### Abstract, Introduction, Conclusion, Achknowledgement ~ 5-10 P. 
- [ ] Acknowledgements
- [ ] Abstract
- [ ] Introduction
- [ ] Conclusions

#### Done
**Graphs ~ 22 P.**  
- [x] Definitions ~ 2 P
- [x] Data structures ~ 5.5 P.
- [x] traversal algos 
- [x] Shortest path algos
- [x] KL & Multilevel & Louvain

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

### Notes
- move analysis of non-adj & incidence list to appendix
- Include permutation, inversion, min-cut in perliminaries?
- Include complexity analysis for g-store/icbl?
- Some feedback on problem definition
- Which figures to draw

## Implementation
<p style="text-align: center;">
<progress id="file" value="0" max="10">Implementation</progress>
</p>  

- [ ] Debug data structures
- [ ] Debug Access
- [ ] Debug ICBL
- [ ] Debug G-Store
- [ ] Debug Queries
- [ ] Record remapping vertices
- [ ] Record remapping relationships
- [ ] Incidence List Reordering
- [ ] Adapt louvain to problem
- [ ] Evaluation main

#### Done
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
- [x] G-Store  [Steinhaus, Olteanu].
- [x] ICBL [Yasar, Gedik]



## Future Improvements
### Implementation Features
- [ ] Use initial community detection + data access history graph to update order subsequently based on queries (Paper)
- [ ] IO
- [ ] Cache
- [ ] __DOCUMENTATION__
- [ ] Tests
- [ ] BSD-style data structures (to avoid wrapper code)
- [ ] Drop consecutive index assumptions & support deletions
- [ ] Alternative record layouts (nodes + adj list in same file)
- [ ] Distributed
- [ ] Multi-Model
- [ ] Transactions/MVCC using git-like transaction logs
