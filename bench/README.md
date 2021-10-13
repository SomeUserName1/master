# Benchmark
## Neo4J
To build and execute the benchmark run ```mvn clean compile exec:java``` from the neo4j folder. 

## GOEDB
Build the whole system as instructed in the main README. From the build directory then execute ```./bench/goedb/benchmark```.

## Comments
Neo4J requires all operations to be wrapped in a trasaction.
For insertions and deletion it maintains full text indices for labels and relationship types.
This comparison does not match 1:1. 
In Neo4J one can find Nodes by labels, which is what I've used for reads, since I don't know exactly how they handle their internal IDs and how that is put together.
This is done by an Index on Label using Lucene.
The find nodes function of the goedb is linear in runtime. 
I've used read_nodes for the benchmark as I know about how IDs are composed. 
I.e. the comparison is not really fair, due to a lack of time for either a) reading through Neo4Js internal APIs or to b) some how index the labels in goedb.
GetNodes and GetRelationships are both a lot faster. 
If i had to guess, I'd say that's due to bulk operators and prefetching.

## Results
All results were gathered by calling the function of interest 15000 times and measuring the call as a whole. The temporal unit are micro seconds.
### Create
**GOEDB**
```
Nodes 0 S. Average call takes 0.556400 mu s  
Edges 0 S. Average call takes 2.300500 mu s
 ```
**Neo4J**
```
Nodes 1 M 0 S. Average call takes 4030mu s 
Edges 1 M 49S. Average call takes 7327mu s 

```

### Read


**GOEDB**
```
Nodes 0 S. Average call takes 0.235400 mu s
Edges 0 S. Average call takes 0.352100 mu s
 ```
 **Neo4J**
 ```
Nodes 0 S. Average call takes 5mu s 
Edges 0 S. Average call takes 2mu s 

```

### Update
**GOEDB**
```
Nodes 0 S. Average call takes 0.207900 mu s
Edges 0 S. Average call takes 0.239000 mu s
 ```
 **Neo4J**
 ```
Nodes 0 S. Average call takes 9mu s 
Relationship can not be updated besides their properties!
```


### Delete
**GOEDB**
```
Nodes 0 S. Average call takes 2.905200 mu s
Edges 0 S. Average call takes 2.631600 mu s
 ```
 **Neo4J**
 ```
Nodes 0 S. Average call takes 18mu s 
Edges 0M 0 S. Average call takes 46mu s 
 ```

### Get\_Nodes
```
GOEDB 23 S. Average call takes 2381.439209 mu s
Neo4J 16 S. Average call takes 1102mu s 
 ```

### Get\_Relationships
```
GOEDB 30 S. Average call takes 3013.833984 mu s
Neo4J 21 S. Average call takes 1396mu s 

 ```

### Expand
```
GOEDB 0 S. Average call takes 1.823900 mu s
Neo4J 32S. Average call takes 2127mu s 
 ```

