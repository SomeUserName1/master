# Master Thesis:  Locality Optimization for traversal-based queries on graph databases

Master Project at the Database and Information Sytems Group, supervised by Dr. Theodoros Chondrogiannis and Prof. Michael Grossniklaus. Dynamic data locality optimizing storage scheme for graph databases.  

## Formalities
- [x] Master Thesis Registration: Last handing in date is the 25th of May 2021  
- [ ] Hand-in Thesis: 16.04
- [ ] Register Colloquium

## Timetable
<p style="text-align: center;">
<progress id="time" value="2" max="26">Time</progress>



| Mon  	| Tue  	| Wed  	| Thu  	| Fri  	| Sat  	| Sun  	|
|---	|---	|---	|---	|---	|---	|---	|
|22.3  :ballot_box_with_check: | 23.3 :pencil: ICBL   |  24.3 :computer: ICBL 	|   25.3 :computer: ICBL 	|   26.3 :pencil: G-Store	|  27.3 :computer: G-Store 	| 28.3 :computer: remap IDs & Debugging  	|
|  29.3 :pencil: Own Method: Louvain w. Adaptions 	| 30.3 :pencil: block ordering   | 31.3 :pencil: Own Method Incidence List reord.  	| 01.4 :computer: Adapt Louvain |  02.4  :computer: block ordering 	| 03.4 :computer: Rearrange Incidence list 	| 04.4 :computer: debugging  	|
| 05.4 :pencil: Setup & Impl.  	| 06.04  :pencil: Data Sets, Queries 	|  07.4 :computer: write code for exp.	| 08.4 :computer: run experiments & generate plots  	| 09.4 :pencil: result discussion   | 10.4  :floppy_disk: 	|  11.4 :floppy_disk: 	|
| 12.4 :pencil: Introduction & Conclusion  	| 13.04 :pencil: Abstract & Figures  & Ack  	| 14.4 :pencil: Proof reading & layout  	|  15.4 :pencil: Proof reading, layout, printing 	|  16.4 :dart: Thesis submission 	| 17.4 :beers:  	|   18.4 :beers:	|
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
<progress id="write" value="2" max="7">Thesis</progress>
</p>

#### Introduction ~ 2 P. 
- [ ] Essay ~ 1 P.
- [ ] brief problem statement ~ 0.3 P.
- [ ] Organization ~ 0.3 P.
- [ ] Contributions! ~ 0.3 P.

#### Related Work ~ 5 P.
- [ ] G-Store
- [ ] ICBL

#### Method ~ 5 P.
- [ ] Louvain + customizations
- [ ] Block ordering
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
- [ ] Abstract
- [ ] Bibliography

**Overall 55 P. Contents + 5-10 P. Title, TOC, Abstract, Bibliography --> 60P.**

#### Done
#### Background ~ 22 P.
- [x] Low-level Database Arch. ~ 3 P.
- [x] Graphs ~ 2 P
- [x] Data structures ~ 5.5 P.
- [x] traversal-based algos ~ 5 P.
- [x] Property Graph Model ~ 1.5 P.
- [x] Neo4J & Index-free Incidence List ~ 5 P.

#### Problem Def. ~ 5 P.
- [x] Locality ~ 2 P.
- [x] Problem definition ~ 0.5 P.
- [x] Example ~ 2 P.

## Implementation
<p style="text-align: center;">
<progress id="file" value="0" max="7">Implementation</progress>
</p>  

- [-] G-Store  [Steinhaus, Olteanu].
- [-] ICBL [Yasar, Gedik]
- [ ] Record remapping
- [ ] Adapt louvain to problem
- [ ] Reorder blocks
- [ ] Incidence List Reordering
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
